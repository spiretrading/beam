#include "Beam/Python/Tasks.hpp"
#include <boost/function_types/components.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/mpl/insert.hpp>
#include "Beam/Tasks/AggregateTask.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/IdleTask.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/PythonTaskFactory.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/Vector.hpp"

using namespace Beam;
using namespace Beam::Python;
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
  struct TaskWrapper : Task, wrapper<Task> {
    virtual void Execute() override {
      this->get_override("execute")();
    }

    virtual void Cancel() override {
      this->get_override("cancel")();
    }

    virtual const Publisher<Task::StateEntry>& GetPublisher() const override {
      return *static_cast<const Publisher<Task::StateEntry>*>(
        this->get_override("get_publisher")());
    }
  };

  struct BasicTaskWrapper : BasicTask, wrapper<BasicTask> {
    virtual void OnExecute() override {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      this->get_override("on_execute")();
    }

    virtual void OnCancel() override {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      this->get_override("on_cancel")();
    }

    void SetActive() {
      BasicTask::SetActive();
    }

    void SetActive(const string& message) {
      BasicTask::SetActive(message);
    }

    void SetTerminal() {
      BasicTask::SetTerminal();
    }

    void SetTerminal(const StateEntry& state) {
      BasicTask::SetTerminal(state);
    }

    void SetTerminal(State state) {
      BasicTask::SetTerminal(state);
    }

    void SetTerminal(State state, const string& message) {
      BasicTask::SetTerminal(state, message);
    }

    void Manage(const std::shared_ptr<Task>& task) {
      BasicTask::Manage(task);
    }
  };

  struct PythonTask : Task {
    PyObject* m_object;
    std::shared_ptr<Task> m_task;

    PythonTask(PyObject& o)
        : m_object{&o} {
      Py_IncRef(m_object);
      Py_IncRef(m_object);
      m_task = extract<std::shared_ptr<Task>>(object{handle<>(m_object)});
    }

    ~PythonTask() {
      Py_DecRef(m_object);
      Py_DecRef(m_object);
      m_task.reset();
    }

    virtual void Execute() override {
      m_task->Execute();
    }

    virtual void Cancel() override {
      m_task->Cancel();
    }

    virtual const Publisher<StateEntry>& GetPublisher() const override {
      return m_task->GetPublisher();
    }
  };

  struct PythonTaskFactoryWrapper : PythonTaskFactory,
      wrapper<PythonTaskFactory> {
    virtual void* Clone() const override {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      auto wrapper = new PythonTaskFactoryWrapper{*this};
      auto pyObject = boost::python::detail::wrapper_base_::get_owner(*wrapper);
      Py_IncRef(pyObject);
      return wrapper;
    }

    virtual std::shared_ptr<Task> Create() override {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      object result = this->get_override("create")();
      return std::make_shared<PythonTask>(*result.ptr());
    }

    boost::python::object FindPythonProperty(const std::string& name) {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      auto& property = PythonTaskFactory::FindProperty(name);
      return *any_cast<boost::python::object>(&property);
    }

    virtual void PrepareContinuation(const Task& task) override {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      if(auto f = this->get_override("prepare_continuation")) {
        f();
        return;
      }
      PythonTaskFactory::PrepareContinuation(task);
    }

    void DefaultPrepareContinuation(const Task& task) {
      this->PythonTaskFactory::PrepareContinuation(task);
    }

    void DefineProperty(const string& name,
        const boost::python::object& value) {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      PythonTaskFactory::DefineProperty(name, value);
    }
  };

  TaskFactory MakeCloneableTaskFactory(const VirtualTaskFactory& factory) {
    return TaskFactory{factory};
  }
}

void Beam::Python::ExportAggregateTask() {
  class_<AggregateTask, std::shared_ptr<AggregateTask>,
    boost::noncopyable, bases<BasicTask>>("AggregateTask",
    init<const vector<TaskFactory>&>());
  class_<AggregateTaskFactory, boost::noncopyable, bases<VirtualTaskFactory>>(
    "AggregateTaskFactory", init<const vector<TaskFactory>&>())
    .def("create", &AggregateTaskFactory::Create)
    .def("prepare_continuation", &AggregateTaskFactory::PrepareContinuation);
  implicitly_convertible<AggregateTaskFactory, TaskFactory>();
  implicitly_convertible<std::shared_ptr<AggregateTask>,
    std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<AggregateTask>,
    std::shared_ptr<Task>>();
}

void Beam::Python::ExportBasicTask() {
  class_<BasicTaskWrapper, std::shared_ptr<BasicTaskWrapper>,
    boost::noncopyable, bases<Task>>("BasicTask")
    .def("execute", BlockingFunction(&BasicTask::Execute))
    .def("cancel", BlockingFunction(&BasicTask::Cancel))
    .def("get_publisher", &BasicTask::GetPublisher,
      return_internal_reference<>())
    .def("on_execute", pure_virtual(&BasicTaskWrapper::OnExecute))
    .def("on_cancel", pure_virtual(&BasicTaskWrapper::OnCancel))
    .def("set_active",
      static_cast<void (BasicTaskWrapper::*)()>(&BasicTaskWrapper::SetActive))
    .def("set_active",
      static_cast<void (BasicTaskWrapper::*)(const string&)>(
      &BasicTaskWrapper::SetActive))
    .def("set_terminal",
      static_cast<void (BasicTaskWrapper::*)()>(&BasicTaskWrapper::SetTerminal))
    .def("set_terminal",
      static_cast<void (BasicTaskWrapper::*)(const Task::StateEntry&)>(
      &BasicTaskWrapper::SetTerminal))
    .def("set_terminal",
      static_cast<void (BasicTaskWrapper::*)(Task::State)>(
      &BasicTaskWrapper::SetTerminal))
    .def("set_terminal",
      static_cast<void (BasicTaskWrapper::*)(Task::State, const string&)>(
      &BasicTaskWrapper::SetTerminal))
    .def("manage", &BasicTaskWrapper::Manage);
  register_ptr_to_python<std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<BasicTaskWrapper>,
    std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<BasicTaskWrapper>,
    std::shared_ptr<Task>>();
  implicitly_convertible<std::shared_ptr<BasicTask>, std::shared_ptr<Task>>();
}

void Beam::Python::ExportIdleTask() {
  class_<IdleTask, std::shared_ptr<IdleTask>, boost::noncopyable,
    bases<BasicTask>>("IdleTask", init<>());
  class_<IdleTaskFactory, boost::noncopyable, bases<VirtualTaskFactory>>(
    "IdleTaskFactory", init<>())
    .def("create", &IdleTaskFactory::Create)
    .def("prepare_continuation", &IdleTaskFactory::PrepareContinuation);
  implicitly_convertible<IdleTaskFactory, TaskFactory>();
  implicitly_convertible<std::shared_ptr<IdleTask>,
    std::shared_ptr<BasicTask>>();
  implicitly_convertible<std::shared_ptr<IdleTask>, std::shared_ptr<Task>>();
}

void Beam::Python::ExportTask() {
  {
    scope outer = class_<TaskWrapper, std::shared_ptr<TaskWrapper>,
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
  implicitly_convertible<std::shared_ptr<TaskWrapper>, std::shared_ptr<Task>>();
}

void Beam::Python::ExportTaskFactory() {
  class_<VirtualTaskFactory, boost::noncopyable>("VirtualTaskFactory", no_init)
    .def("create", pure_virtual(&VirtualTaskFactory::Create));
  class_<PythonTaskFactoryWrapper, boost::noncopyable,
    bases<VirtualTaskFactory>>("TaskFactory")
    .def("__copy__", &MakeCopy<PythonTaskFactoryWrapper>)
    .def("__deepcopy__", &MakeDeepCopy<PythonTaskFactoryWrapper>)
    .def("find_property", &PythonTaskFactoryWrapper::FindPythonProperty)
    .def("prepare_continuation", &PythonTaskFactory::PrepareContinuation,
      &PythonTaskFactoryWrapper::DefaultPrepareContinuation)
    .def("get", &PythonTaskFactory::Get)
    .def("set", &PythonTaskFactory::Set)
    .def("define_property", &PythonTaskFactoryWrapper::DefineProperty);
  class_<TaskFactory>("CloneableTaskFactory", no_init)
    .def("__init__", &MakeCloneableTaskFactory);
  ExportVector<vector<TaskFactory>>("TaskFactoryVector");
  implicitly_convertible<VirtualTaskFactory, TaskFactory>();
  implicitly_convertible<PythonTaskFactoryWrapper, TaskFactory>();
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
  ExportIdleTask();
  def("is_terminal", &IsTerminal);
  def("wait", BlockingFunction(&Wait));
}
