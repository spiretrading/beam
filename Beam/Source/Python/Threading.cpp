#include "Beam/Python/Threading.hpp"
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/ThreadPool.hpp"
#include "Beam/Threading/TimeoutException.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  auto timerBox = std::unique_ptr<class_<TimerBox>>();
}

class_<TimerBox>& Beam::Python::GetExportedTimerBox() {
  return *timerBox;
}

void Beam::Python::ExportConditionVariable(module& module) {
  class_<ConditionVariable>(module, "ConditionVariable").
    def(init()).
    def("wait",
      [] (ConditionVariable& self, Mutex& m) {
        auto lock = std::unique_lock(m, std::try_to_lock);
        self.wait(lock);
      }, call_guard<GilRelease>()).
    def("notify_one", &ConditionVariable::notify_one, call_guard<GilRelease>()).
    def("notify_all", &ConditionVariable::notify_all,
      call_guard<GilRelease>());
}

void Beam::Python::ExportLiveTimer(module& module) {
  ExportTimer<ToPythonTimer<LiveTimer>>(module, "LiveTimer").
    def(init<time_duration>());
}

void Beam::Python::ExportMutex(module& module) {
  class_<Mutex>(module, "Mutex").
    def(init()).
    def("lock", &Mutex::lock, call_guard<GilRelease>()).
    def("try_lock", &Mutex::try_lock, call_guard<GilRelease>()).
    def("unlock", &Mutex::unlock, call_guard<GilRelease>()).
    def("__enter__",
      [] (object& self) {
        self.attr("lock")();
        return self;
      }).
    def("__exit__",
      [] (object& self) {
        self.attr("unlock")();
      });
}

void Beam::Python::ExportRecursiveMutex(module& module) {
  class_<RecursiveMutex>(module, "RecursiveMutex").
    def(init()).
    def("lock", &RecursiveMutex::lock, call_guard<GilRelease>()).
    def("try_lock", &RecursiveMutex::try_lock, call_guard<GilRelease>()).
    def("unlock", &RecursiveMutex::unlock, call_guard<GilRelease>()).
    def("__enter__",
      [] (object& self) {
        self.attr("lock")();
        return self;
      }).
    def("__exit__",
      [] (object& self) {
        self.attr("unlock")();
      });
}

void Beam::Python::ExportThreading(module& module) {
  auto submodule = module.def_submodule("threading");
  ExportConditionVariable(submodule);
  ExportMutex(submodule);
  ExportRecursiveMutex(submodule);
  timerBox = std::make_unique<class_<TimerBox>>(ExportTimer<TimerBox>(
    submodule, "Timer"));
  ExportTimer<ToPythonTimer<TimerBox>>(submodule, "TimerBox");
  ExportLiveTimer(submodule);
  ExportTriggerTimer(submodule);
  ExportQueueSuite<Timer::Result>(module, "TimerResult");
  module.def("park", [] (const std::function<void ()>& f) {
    Park(f);
  }, call_guard<GilRelease>());
  register_exception<TimeoutException>(submodule, "TimeoutException");
}

void Beam::Python::ExportTriggerTimer(module& module) {
  ExportTimer<ToPythonTimer<TriggerTimer>>(module, "TriggerTimer").
    def(init()).
    def("trigger",
      [] (ToPythonTimer<TriggerTimer>& self) {
        self.GetTimer().Trigger();
      }).
    def("fail",
      [] (ToPythonTimer<TriggerTimer>& self) {
        self.GetTimer().Fail();
      });
}
