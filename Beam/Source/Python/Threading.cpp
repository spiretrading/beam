#include "Beam/Python/Threading.hpp"
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/Threading/LiveTimer.hpp"
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

void Beam::Python::ExportLiveTimer(module& module) {
  ExportTimer<ToPythonTimer<LiveTimer>>(module, "LiveTimer").
    def(init<time_duration>());
}

void Beam::Python::ExportThreading(module& module) {
  auto submodule = module.def_submodule("threading");
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
