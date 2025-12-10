#include "Beam/Python/Threading.hpp"
#include "Beam/Python/PythonFunction.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/TaskRunner.hpp"
#include "Beam/Threading/ThreadPool.hpp"
#include "Beam/Threading/TimedConditionVariable.hpp"
#include "Beam/Threading/TimeoutException.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost::posix_time;
using namespace pybind11;

void Beam::Python::export_condition_variable(module& module) {
  class_<ConditionVariable>(module, "ConditionVariable").
    def(pybind11::init()).
    def("wait", [] (ConditionVariable& self, Mutex& m) {
      auto lock = std::unique_lock(m, std::try_to_lock);
      self.wait(lock);
    }, call_guard<gil_scoped_release>()).
    def("notify_one", &ConditionVariable::notify_one,
      call_guard<gil_scoped_release>()).
    def("notify_all", &ConditionVariable::notify_all,
      call_guard<gil_scoped_release>());
}

void Beam::Python::export_mutex(module& module) {
  class_<Mutex>(module, "Mutex").
    def(pybind11::init()).
    def("lock", &Mutex::lock, call_guard<gil_scoped_release>()).
    def("try_lock", &Mutex::try_lock, call_guard<gil_scoped_release>()).
    def("unlock", &Mutex::unlock, call_guard<gil_scoped_release>()).
    def("__enter__", [] (object& self) {
      self.attr("lock")();
      return self;
    }).
    def("__exit__", [] (object& self) {
      self.attr("unlock")();
    });
}

void Beam::Python::export_recursive_mutex(module& module) {
  class_<RecursiveMutex>(module, "RecursiveMutex").
    def(pybind11::init()).
    def("lock", &RecursiveMutex::lock, call_guard<gil_scoped_release>()).
    def("try_lock", &RecursiveMutex::try_lock,
      call_guard<gil_scoped_release>()).
    def("unlock", &RecursiveMutex::unlock, call_guard<gil_scoped_release>()).
    def("__enter__", [] (object& self) {
      self.attr("lock")();
      return self;
    }).
    def("__exit__", [] (object& self) {
      self.attr("unlock")();
    });
}

void Beam::Python::export_task_runner(module& module) {
  class_<TaskRunner>(module, "TaskRunner").
    def(pybind11::init()).
    def("add", [] (TaskRunner& self, const PythonFunction<void ()>& task) {
      self.add(task);
    }).
    def_property_readonly("is_empty", &TaskRunner::is_empty);
}

void Beam::Python::export_thread_pool(module& module) {
  module.def("park", [] (const PythonFunction<void ()>& f) {
    park(f);
  }, call_guard<gil_scoped_release>());
}

void Beam::Python::export_timed_condition_variable(module& module) {
  class_<TimedConditionVariable>(module, "TimedConditionVariable").
    def(pybind11::init()).
    def("wait", [] (TimedConditionVariable& self, Mutex& m) {
      auto lock = std::unique_lock(m, std::try_to_lock);
      self.wait(lock);
    }, call_guard<gil_scoped_release>()).
    def("timed_wait", [] (TimedConditionVariable& self, time_duration duration,
        Mutex& m) {
      auto lock = std::unique_lock(m, std::try_to_lock);
      self.timed_wait(duration, lock);
    }, call_guard<gil_scoped_release>()).
    def("notify_one", &TimedConditionVariable::notify_one,
      call_guard<gil_scoped_release>()).
    def("notify_all", &TimedConditionVariable::notify_all,
      call_guard<gil_scoped_release>());
}

void Beam::Python::export_threading(module& module) {
  export_condition_variable(module);
  export_mutex(module);
  export_recursive_mutex(module);
  export_sync<object>(module, "Sync");
  export_task_runner(module);
  export_thread_pool(module);
  export_timed_condition_variable(module);
  register_exception<TimeoutException>(module, "TimeoutException");
}
