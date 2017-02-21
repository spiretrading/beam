#include "Beam/Python/Tasks.hpp"
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/PythonTaskFactory.hpp"
#include "Beam/Python/Queues.hpp"

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

  struct PythonTaskFactoryWrapper : PythonTaskFactory,
      wrapper<PythonTaskFactory>, CloneableMixin<PythonTaskFactoryWrapper> {
    virtual std::shared_ptr<Task> Create() override {
      return this->get_override("create")();
    }

    boost::python::object FindPythonProperty(const std::string& name) {
      auto& property = PythonTaskFactory::FindProperty(name);
      return *any_cast<boost::python::object>(&property);
    }

    virtual void PrepareContinuation(const Task& task) override {
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
      PythonTaskFactory::DefineProperty(name, value);
    }
  };

  TaskFactory MakeCloneableTaskFactory(const VirtualTaskFactory& factory) {
    return TaskFactory{factory};
  }
}

void Beam::Python::ExportBasicTask() {
  class_<BasicTaskWrapper, std::shared_ptr<BasicTaskWrapper>,
    boost::noncopyable, bases<Task>>("BasicTask")
    .def("execute", BlockingFunction(&BasicTaskWrapper::Execute))
    .def("cancel", BlockingFunction(&BasicTaskWrapper::Cancel))
    .def("get_publisher", &BasicTaskWrapper::GetPublisher,
      return_value_policy<reference_existing_object>())
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
}

void Beam::Python::ExportTask() {
  {
    scope outer = class_<TaskWrapper, std::shared_ptr<TaskWrapper>,
      boost::noncopyable>("Task")
      .add_property("id", &Task::GetId)
      .def("execute", pure_virtual(&Task::Execute))
      .def("cancel", pure_virtual(&Task::Cancel))
      .def("get_publisher", pure_virtual(&Task::GetPublisher),
        return_value_policy<reference_existing_object>());
    enum_<Task::State::Type>("State")
      .value("NONE", Task::State::NONE)
      .value("INITIALIZING", Task::State::INITIALIZING)
      .value("ACTIVE", Task::State::ACTIVE)
      .value("PENDING_CANCEL", Task::State::PENDING_CANCEL)
      .value("CANCELED", Task::State::CANCELED)
      .value("FAILED", Task::State::FAILED)
      .value("EXPIRED", Task::State::EXPIRED)
      .value("COMPLETE", Task::State::COMPLETE);
    class_<Task::StateEntry>("StateEntry", init<>())
      .def(init<Task::State>())
      .def(init<Task::State, const string&>())
      .def_readwrite("state", &Task::StateEntry::m_state)
      .def_readwrite("message", &Task::StateEntry::m_message);
  }
}

void Beam::Python::ExportTaskFactory() {
  class_<PythonTaskFactoryWrapper, boost::noncopyable>("TaskFactory")
    .def("create", pure_virtual(&PythonTaskFactory::Create))
    .def("find_property", &PythonTaskFactoryWrapper::FindPythonProperty)
    .def("prepare_continuation", &PythonTaskFactory::PrepareContinuation,
      &PythonTaskFactoryWrapper::DefaultPrepareContinuation)
    .def("get", &PythonTaskFactory::Get)
    .def("set", &PythonTaskFactory::Set)
    .def("define_property", &PythonTaskFactoryWrapper::DefineProperty);
  class_<TaskFactory>("CloneableTaskFactory", no_init)
    .def("__init__", &MakeCloneableTaskFactory);
  class_<vector<TaskFactory>>("TaskFactoryVector")
    .def(vector_indexing_suite<vector<TaskFactory>>());
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
  def("is_terminal", &IsTerminal);
  def("wait", BlockingFunction(&Wait));
}
