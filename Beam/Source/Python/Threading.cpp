#include <condition_variable>
#include <pybind11/chrono.h>
#include "Beam/Python/Threading.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonFunction.hpp"
#include "Beam/Threading/TaskRunner.hpp"
#include "Beam/Threading/ThreadPool.hpp"
#include "Beam/Threading/TimeoutException.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

void Beam::Python::export_condition_variable(module& module) {
  class_<std::condition_variable>(module, "ConditionVariable").
    def(pybind11::init()).
    def("wait", [] (std::condition_variable& self, std::mutex& mutex) {
      auto lock = std::unique_lock(mutex, std::adopt_lock);
      self.wait(lock);
      lock.release();
    }, call_guard<GilRelease>()).
    def("wait_for", [] (std::condition_variable& self, std::mutex& mutex,
        std::chrono::microseconds duration) {
      auto lock = std::unique_lock(mutex, std::adopt_lock);
      auto status = self.wait_for(lock, duration);
      lock.release();
      return status == std::cv_status::no_timeout;
    }, call_guard<GilRelease>()).
    def("notify_one", &std::condition_variable::notify_one,
      call_guard<GilRelease>()).
    def("notify_all", &std::condition_variable::notify_all,
      call_guard<GilRelease>());
}

void Beam::Python::export_mutex(module& module) {
  class_<std::mutex>(module, "Mutex").
    def(pybind11::init()).
    def("lock", &std::mutex::lock, call_guard<GilRelease>()).
    def("try_lock", &std::mutex::try_lock, call_guard<GilRelease>()).
    def("unlock", &std::mutex::unlock, call_guard<GilRelease>()).
    def("__enter__", [] (object& self) {
      self.attr("lock")();
      return self;
    }).
    def("__exit__", [] (
        object& self, const object&, const object&, const object&) {
      self.attr("unlock")();
    });
}

void Beam::Python::export_recursive_mutex(module& module) {
  class_<std::recursive_mutex>(module, "RecursiveMutex").
    def(pybind11::init()).
    def("lock", &std::recursive_mutex::lock, call_guard<GilRelease>()).
    def("try_lock", &std::recursive_mutex::try_lock, call_guard<GilRelease>()).
    def("unlock", &std::recursive_mutex::unlock, call_guard<GilRelease>()).
    def("__enter__", [] (object& self) {
      self.attr("lock")();
      return self;
    }).
    def("__exit__", [] (
        object& self, const object&, const object&, const object&) {
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
  }, call_guard<GilRelease>());
}

void Beam::Python::export_threading(module& module) {
  export_condition_variable(module);
  export_mutex(module);
  export_recursive_mutex(module);
  export_sync<object>(module, "Sync");
  export_task_runner(module);
  export_thread_pool(module);
  register_exception<TimeoutException>(module, "TimeoutException");
}
