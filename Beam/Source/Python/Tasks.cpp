#include "Beam/Python/Tasks.hpp"
#include "Beam/Tasks/AggregateTask.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/ChainedTask.hpp"
#include "Beam/Tasks/IdleTask.hpp"
#include "Beam/Tasks/ReactorMonitorTask.hpp"
#include "Beam/Tasks/ReactorTask.hpp"
#include "Beam/Tasks/SpawnTask.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/Tasks/TaskPropertyNotFoundException.hpp"
#include "Beam/Tasks/UntilTask.hpp"
#include "Beam/Tasks/WhenTask.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/FromPythonReactor.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/PythonPackagedTask.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/ToPythonTask.hpp"
#include "Beam/Python/Vector.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace Beam::Tasks;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace Beam {
namespace Tasks {
  bool operator ==(const TaskFactory& lhs, const TaskFactory& rhs) {
    return &*lhs == &*rhs;
  }
}
}

namespace {
  struct FromPythonTask : Task, wrapper<Task> {
    virtual ~FromPythonTask() override final = default;

    virtual void Execute() override final {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      get_override("execute")();
    }

    virtual void Cancel() override final {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      get_override("cancel")();
    }

    virtual const Publisher<Task::StateEntry>&
        GetPublisher() const override final {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      return *static_cast<const Publisher<Task::StateEntry>*>(
        get_override("get_publisher")());
    }
  };

  struct FromPythonTaskFactory : VirtualTaskFactory,
      wrapper<VirtualTaskFactory>, CloneableMixin<FromPythonTaskFactory> {
    FromPythonTaskFactory() = default;

    FromPythonTaskFactory(const FromPythonTaskFactory& other)
        : VirtualTaskFactory{static_cast<const VirtualTaskFactory&>(other)},
          wrapper<VirtualTaskFactory>{
            static_cast<const wrapper<VirtualTaskFactory>&>(other)} {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      m_properties = other.m_properties;
      auto pyObject = boost::python::detail::wrapper_base_::get_owner(*this);
      Py_IncRef(pyObject);
    }

    virtual ~FromPythonTaskFactory() override final = default;

    void DefineProperty(const std::string& name,
        const boost::python::object& value) {
      m_properties.insert(std::make_pair(name, value));
    }

    virtual std::shared_ptr<Task> Create() {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      return get_override("create")();
    }

    virtual boost::any& FindProperty(const std::string& name) override final {
      auto propertyIterator = m_properties.find(name);
      if(propertyIterator == m_properties.end()) {
        BOOST_THROW_EXCEPTION(TaskPropertyNotFoundException{name});
      }
      return propertyIterator->second;
    }

    virtual void PrepareContinuation(const Task& task) {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      if(auto f = get_override("prepare_continuation")) {
        f(task);
        return;
      }
      VirtualTaskFactory::PrepareContinuation(task);
    }

    void DefaultPrepareContinuation(const Task& task) {
      VirtualTaskFactory::PrepareContinuation(task);
    }

    std::unordered_map<std::string, boost::any> m_properties;
  };

  boost::python::object FromPythonTaskFactoryGet(VirtualTaskFactory& factory,
      const std::string& name) {
    return GetTaskFactoryProperty(&factory, name.c_str());
  }

  void FromPythonTaskFactorySet(VirtualTaskFactory& factory,
      const std::string& name, const boost::python::object& value) {
    SetTaskFactoryProperty(&factory, name.c_str(), &value);
  }

  struct FromPythonBasicTask : BasicTask, wrapper<BasicTask> {
    virtual void OnExecute() override final {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      get_override("on_execute")();
    }

    virtual void OnCancel() override final {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      get_override("on_cancel")();
    }

    void SetActive() {
      BasicTask::SetActive();
    }

    void SetActive(const std::string& message) {
      BasicTask::SetActive(message);
    }

    void SetTerminal() {
      BasicTask::SetTerminal();
    }

    void SetTerminal(const StateEntry& state) {
      BasicTask::SetTerminal(state);
    }

    void SetTerminal(State state, const std::string& message) {
      BasicTask::SetTerminal(state, message);
    }

    void Manage(std::shared_ptr<Task> task) {
      BasicTask::Manage(std::move(task));
    }
  };

  class PythonReactorProperty : public VirtualReactorProperty,
      public CloneableMixin<PythonReactorProperty> {
    public:
      PythonReactorProperty(std::string name,
          std::shared_ptr<Reactor<boost::python::object>> reactor)
          : VirtualReactorProperty(std::move(name)),
            m_reactor{MakeFromPythonReactor<boost::python::object>(
              MakeNonRepeatingReactor(std::move(reactor)))} {}

      virtual std::shared_ptr<BaseReactor> GetReactor() const override final {
        return m_reactor;
      }

      virtual void Commit() override final {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        m_value = m_reactor->Eval();
      }

      virtual void Apply(TaskFactory& factory) const override final {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          SetTaskFactoryProperty(&*factory, GetName().c_str(),
            m_value.get_ptr());
        } catch(const TaskPropertyNotFoundException&) {}
      }

    private:
      std::shared_ptr<Reactor<boost::python::object>> m_reactor;
      boost::optional<boost::python::object> m_value;
  };

  boost::python::object MakePythonPackagedTaskFactory(
      const boost::python::tuple& args, const boost::python::dict& kw) {
    boost::python::object self = args[0];
    boost::python::object package = args[1];
    boost::python::tuple a = boost::python::tuple(
      args.slice(2, boost::python::_));
    return self.attr("__init__")(package, a, kw);
  }

  void PythonPackagedTaskFactorySet(PythonPackagedTaskFactory& factory,
      const std::string& name, const boost::python::object& value) {
    factory.Set(name, value);
  }

  auto MakePythonReactorTask(ReactorMonitor& reactorMonitor,
      const boost::python::object& properties, TaskFactory taskFactory) {
    std::vector<ReactorProperty> p;
    for(int i = 0; i < boost::python::len(properties); ++i) {
      auto& rp = boost::python::extract<VirtualReactorProperty&>{
        properties[i]}();
      p.push_back(std::move(rp));
    }
    return std::make_shared<ToPythonTask<ReactorTask>>(Ref(reactorMonitor),
      std::move(p), std::move(taskFactory));
  }

  auto MakePythonReactorTaskFactory(ReactorMonitor& reactorMonitor,
      const boost::python::object& properties, TaskFactory taskFactory) {
    std::vector<ReactorProperty> p;
    for(int i = 0; i < boost::python::len(properties); ++i) {
      auto& rp = boost::python::extract<VirtualReactorProperty&>{
        properties[i]}();
      p.push_back(std::move(rp));
    }
    return new ToPythonTaskFactory<ReactorTaskFactory>{Ref(reactorMonitor),
      std::move(p), std::move(taskFactory)};
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<AggregateTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<AggregateTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(BasicTask);
BEAM_DEFINE_PYTHON_POINTER_LINKER(FromPythonBasicTask);
BEAM_DEFINE_PYTHON_POINTER_LINKER(FromPythonTask);
BEAM_DEFINE_PYTHON_POINTER_LINKER(FromPythonTaskFactory);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<ChainedTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<ChainedTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<IdleTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<IdleTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Publisher<Task::StateEntry>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(PythonPackagedTask);
BEAM_DEFINE_PYTHON_POINTER_LINKER(PythonPackagedTaskFactory);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Task);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<ReactorMonitorTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  ToPythonTaskFactory<ReactorMonitorTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<ReactorTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<ReactorTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<SpawnTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<SpawnTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<UntilTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<UntilTaskFactory>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTask<WhenTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ToPythonTaskFactory<WhenTaskFactory>);

void Beam::Python::ExportAggregateTask() {
  class_<ToPythonTask<AggregateTask>,
    std::shared_ptr<ToPythonTask<AggregateTask>>, boost::noncopyable,
    bases<Task>>("AggregateTask", init<vector<TaskFactory>>());
  class_<ToPythonTaskFactory<AggregateTaskFactory>, bases<VirtualTaskFactory>>(
    "AggregateTaskFactory", init<vector<TaskFactory>>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<AggregateTaskFactory>>)
    .def("__deepcopy__", &MakeDeepCopy<
    ToPythonTaskFactory<AggregateTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<AggregateTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<AggregateTaskFactory>,
    TaskFactory>();
}

void Beam::Python::ExportBasicTask() {
  class_<FromPythonBasicTask, std::shared_ptr<FromPythonBasicTask>,
    boost::noncopyable, bases<Task>>("BasicTask")
    .def("execute", BlockingFunction(&BasicTask::Execute))
    .def("cancel", BlockingFunction(&BasicTask::Cancel))
    .def("on_execute", pure_virtual(&FromPythonBasicTask::OnExecute))
    .def("on_cancel", pure_virtual(&FromPythonBasicTask::OnCancel))
    .def("set_active", static_cast<void (FromPythonBasicTask::*)()>(
      &FromPythonBasicTask::SetActive))
    .def("set_active", static_cast<
      void (FromPythonBasicTask::*)(const string&)>(
      &FromPythonBasicTask::SetActive))
    .def("set_terminal", static_cast<
      void (FromPythonBasicTask::*)()>(&FromPythonBasicTask::SetTerminal))
    .def("set_terminal", static_cast<
      void (FromPythonBasicTask::*)(Task::State, const string&)>(
      &FromPythonBasicTask::SetTerminal))
    .def("set_terminal", static_cast<
      void (FromPythonBasicTask::*)(const Task::StateEntry&)>(
      &FromPythonBasicTask::SetTerminal))
    .def("manage", &FromPythonBasicTask::Manage);
  register_ptr_to_python<std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<FromPythonBasicTask>,
    std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<FromPythonBasicTask>,
    std::shared_ptr<Task>>();
  implicitly_convertible<std::shared_ptr<BasicTask>, std::shared_ptr<Task>>();
}

void Beam::Python::ExportChainedTask() {
  class_<ToPythonTask<ChainedTask>, std::shared_ptr<ToPythonTask<ChainedTask>>,
    boost::noncopyable, bases<Task>>("ChainedTask",
    init<std::vector<TaskFactory>>());
  class_<ToPythonTaskFactory<ChainedTaskFactory>, bases<VirtualTaskFactory>>(
    "ChainedTaskFactory", init<std::vector<TaskFactory>>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<ChainedTaskFactory>>)
    .def("__deepcopy__",
      &MakeDeepCopy<ToPythonTaskFactory<ChainedTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<ChainedTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<ChainedTaskFactory>,
    TaskFactory>();
}

void Beam::Python::ExportIdleTask() {
  class_<ToPythonTask<IdleTask>, std::shared_ptr<ToPythonTask<IdleTask>>,
    boost::noncopyable, bases<Task>>("IdleTask", init<>());
  class_<ToPythonTaskFactory<IdleTaskFactory>, bases<VirtualTaskFactory>>(
    "IdleTaskFactory", init<>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<IdleTaskFactory>>)
    .def("__deepcopy__", &MakeDeepCopy<ToPythonTaskFactory<IdleTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<IdleTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<IdleTaskFactory>, TaskFactory>();
}

void Beam::Python::ExportPythonPackagedTask() {
  class_<PythonFunctionPackage>("FunctionPackage", no_init)
    .def_readwrite("execute", &PythonFunctionPackage::m_execute)
    .def_readwrite("cancel", &PythonFunctionPackage::m_cancel);
  class_<PythonPackagedTask, std::shared_ptr<PythonPackagedTask>,
    boost::noncopyable, bases<BasicTask>>("PackagedTask",
    init<const boost::python::object&, const boost::python::tuple&,
      const boost::python::dict&>());
  class_<PythonPackagedTaskFactory, bases<VirtualTaskFactory>>(
    "PackagedTaskFactory", no_init)
    .def("__init__", raw_function(&MakePythonPackagedTaskFactory, 1))
    .def(init<const boost::python::object&, const boost::python::tuple&,
      const boost::python::dict&>())
    .def("__copy__", &MakeCopy<PythonPackagedTaskFactory>)
    .def("__deepcopy__", &MakeDeepCopy<PythonPackagedTaskFactory>)
    .def("get_parameter_name", &PythonPackagedTaskFactory::GetParameterName,
      return_value_policy<copy_const_reference>())
    .def("set", &PythonPackagedTaskFactorySet)
    .def("create", &PythonPackagedTaskFactory::Create)
    .def("prepare_continuation",
      &PythonPackagedTaskFactory::PrepareContinuation);
  implicitly_convertible<std::shared_ptr<PythonPackagedTask>,
    std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<PythonPackagedTask>,
    std::shared_ptr<Task>>();
  implicitly_convertible<PythonPackagedTaskFactory, TaskFactory>();
  def("make_function_task_factory",
    raw_function(&MakePythonFunctionTaskFactory, 1));
}

void Beam::Python::ExportReactorMonitorTask() {
  class_<ToPythonTask<ReactorMonitorTask>,
    std::shared_ptr<ToPythonTask<ReactorMonitorTask>>, boost::noncopyable,
    bases<Task>>("ReactorMonitorTask", init<RefType<ReactorMonitor>,
    TaskFactory>());
  class_<ToPythonTaskFactory<ReactorMonitorTaskFactory>,
    bases<VirtualTaskFactory>>("ReactorMonitorTaskFactory",
    init<RefType<ReactorMonitor>, TaskFactory>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<ReactorMonitorTaskFactory>>)
    .def("__deepcopy__",
      &MakeDeepCopy<ToPythonTaskFactory<ReactorMonitorTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<ReactorMonitorTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<ReactorMonitorTaskFactory>,
    TaskFactory>();
}

void Beam::Python::ExportReactorTask() {
  class_<ToPythonTask<ReactorTask>, std::shared_ptr<ToPythonTask<ReactorTask>>,
    boost::noncopyable, bases<Task>>("ReactorTask", no_init)
    .def("__init__", make_constructor(&MakePythonReactorTask));
  class_<ToPythonTaskFactory<ReactorTaskFactory>, bases<VirtualTaskFactory>>(
    "ReactorTaskFactory", no_init)
    .def("__init__", make_constructor(&MakePythonReactorTaskFactory))
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<ReactorTaskFactory>>)
    .def("__deepcopy__",
      &MakeDeepCopy<ToPythonTaskFactory<ReactorTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<ReactorTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<ReactorTaskFactory>,
    TaskFactory>();
  class_<VirtualReactorProperty, boost::noncopyable>("BaseReactorProperty",
    no_init)
    .add_property("name", make_function(&VirtualReactorProperty::GetName,
      return_value_policy<copy_const_reference>()))
    .add_property("reactor", &VirtualReactorProperty::GetReactor)
    .def("commit", &VirtualReactorProperty::Commit)
    .def("apply", &VirtualReactorProperty::Apply);
  class_<PythonReactorProperty, bases<VirtualReactorProperty>,
    boost::noncopyable>("ReactorProperty", init<const string&,
    const std::shared_ptr<Reactor<boost::python::object>>>());
  class_<ReactorProperty>("CloneableReactorProperty", no_init)
    .def("__copy__", &MakeCopy<ReactorProperty>)
    .def("__deepcopy__", &MakeDeepCopy<ReactorProperty>);
  implicitly_convertible<VirtualReactorProperty, ReactorProperty>();
  implicitly_convertible<PythonReactorProperty, ReactorProperty>();
}

void Beam::Python::ExportSpawnTask() {
  class_<ToPythonTask<SpawnTask>, std::shared_ptr<ToPythonTask<SpawnTask>>,
    boost::noncopyable, bases<Task>>("SpawnTask", init<RefType<ReactorMonitor>,
    std::shared_ptr<BaseReactor>, TaskFactory>());
  class_<ToPythonTaskFactory<SpawnTaskFactory>, bases<VirtualTaskFactory>>(
    "SpawnTaskFactory", init<RefType<ReactorMonitor>,
    std::shared_ptr<BaseReactor>, TaskFactory>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<SpawnTaskFactory>>)
    .def("__deepcopy__", &MakeDeepCopy<ToPythonTaskFactory<SpawnTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<SpawnTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<SpawnTaskFactory>, TaskFactory>();
}

void Beam::Python::ExportTask() {
  {
    scope outer = class_<FromPythonTask, std::shared_ptr<FromPythonTask>,
      boost::noncopyable>("Task")
      .add_property("id", &Task::GetId)
      .def("execute", pure_virtual(&Task::Execute))
      .def("cancel", pure_virtual(&Task::Cancel))
      .def("get_publisher", pure_virtual(&Task::GetPublisher),
        return_internal_reference<>());
    enum_<Task::State::Type>("State")
      .value("NONE", Task::State::NONE)
      .value("INITIALIZING", Task::State::INITIALIZING)
      .value("ACTIVE", Task::State::ACTIVE)
      .value("PENDING_CANCEL", Task::State::PENDING_CANCEL)
      .value("CANCELED", Task::State::CANCELED)
      .value("FAILED", Task::State::FAILED)
      .value("EXPIRED", Task::State::EXPIRED)
      .value("COMPLETE", Task::State::COMPLETE);
    ExportEnum<Task::State>();
    class_<Task::StateEntry>("StateEntry", init<>())
      .def(init<Task::State>())
      .def(init<Task::State, const string&>())
      .add_property("state",
        make_getter(&Task::StateEntry::m_state,
        return_value_policy<return_by_value>()),
        make_setter(&Task::StateEntry::m_state,
        return_value_policy<return_by_value>()))
      .def_readwrite("message", &Task::StateEntry::m_message);
  }
  ExportPublisher<Task::StateEntry>("TaskStateEntryPublisher");
  register_ptr_to_python<std::shared_ptr<Task>>();
  implicitly_convertible<std::shared_ptr<FromPythonTask>,
    std::shared_ptr<Task>>();
}

void Beam::Python::ExportTaskFactory() {
  class_<FromPythonTaskFactory, boost::noncopyable>("TaskFactory", init<>())
    .def("__copy__", &MakeCopy<FromPythonTaskFactory>)
    .def("__deepcopy__", &MakeDeepCopy<FromPythonTaskFactory>)
    .def("get", &FromPythonTaskFactoryGet)
    .def("set", &FromPythonTaskFactorySet)
    .def("define_property", &FromPythonTaskFactory::DefineProperty)
    .def("create", pure_virtual(&VirtualTaskFactory::Create))
    .def("find_property", &FromPythonTaskFactory::FindProperty,
      return_internal_reference<>())
    .def("prepare_continuation", &VirtualTaskFactory::PrepareContinuation,
      &FromPythonTaskFactory::DefaultPrepareContinuation);
  class_<TaskFactory>("CloneableTaskFactory", init<const VirtualTaskFactory&>())
    .def("__copy__", &MakeCopy<TaskFactory>)
    .def("__deepcopy__", &MakeDeepCopy<TaskFactory>);
  ExportVector<vector<TaskFactory>>("TaskFactoryVector");
  implicitly_convertible<VirtualTaskFactory, TaskFactory>();
  implicitly_convertible<FromPythonTaskFactory, TaskFactory>();
}

void Beam::Python::ExportTasks() {
  string nestedName = extract<string>(scope().attr("__name__") + ".tasks");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("tasks") = nestedModule;
  scope parent = nestedModule;
  ExportTask();
  ExportTaskFactory();
  ExportBasicTask();
  ExportAggregateTask();
  ExportChainedTask();
  ExportIdleTask();
  ExportPythonPackagedTask();
  ExportReactorTask();
  ExportReactorMonitorTask();
  ExportSpawnTask();
  ExportUntilTask();
  ExportWhenTask();
  ExportException<TaskPropertyNotFoundException, std::runtime_error>(
    "TaskPropertyNotFoundException")
    .def(init<const string&>());
  ExportTaskFactoryProperty<bool>();
  ExportTaskFactoryProperty<char>();
  ExportTaskFactoryProperty<unsigned char>();
  ExportTaskFactoryProperty<short>();
  ExportTaskFactoryProperty<unsigned short>();
  ExportTaskFactoryProperty<int>();
  ExportTaskFactoryProperty<unsigned int>();
  ExportTaskFactoryProperty<long>();
  ExportTaskFactoryProperty<unsigned long>();
  ExportTaskFactoryProperty<long long>();
  ExportTaskFactoryProperty<unsigned long long>();
  ExportTaskFactoryProperty<float>();
  ExportTaskFactoryProperty<double>();
  ExportTaskFactoryProperty<std::string>();
  def("is_terminal", &IsTerminal);
  def("wait", BlockingFunction(&Wait));
}

void Beam::Python::ExportUntilTask() {
  class_<ToPythonTask<UntilTask>, std::shared_ptr<ToPythonTask<UntilTask>>,
    boost::noncopyable, bases<Task>>("UntilTask", init<RefType<ReactorMonitor>,
    std::shared_ptr<Reactor<bool>>, TaskFactory>());
  class_<ToPythonTaskFactory<UntilTaskFactory>, bases<VirtualTaskFactory>>(
    "UntilTaskFactory", init<RefType<ReactorMonitor>,
    std::shared_ptr<Reactor<bool>>, TaskFactory>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<UntilTaskFactory>>)
    .def("__deepcopy__", &MakeDeepCopy<ToPythonTaskFactory<UntilTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<UntilTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<UntilTaskFactory>, TaskFactory>();
}

void Beam::Python::ExportWhenTask() {
  class_<ToPythonTask<WhenTask>, std::shared_ptr<ToPythonTask<WhenTask>>,
    boost::noncopyable, bases<Task>>("WhenTask", init<RefType<ReactorMonitor>,
    std::shared_ptr<Reactor<bool>>, TaskFactory>());
  class_<ToPythonTaskFactory<WhenTaskFactory>, bases<VirtualTaskFactory>>(
    "WhenTaskFactory", init<RefType<ReactorMonitor>,
    std::shared_ptr<Reactor<bool>>, TaskFactory>())
    .def("__copy__", &MakeCopy<ToPythonTaskFactory<WhenTaskFactory>>)
    .def("__deepcopy__", &MakeDeepCopy<ToPythonTaskFactory<WhenTaskFactory>>);
  implicitly_convertible<std::shared_ptr<ToPythonTask<WhenTask>>,
    std::shared_ptr<Task>>();
  implicitly_convertible<ToPythonTaskFactory<WhenTaskFactory>, TaskFactory>();
}
