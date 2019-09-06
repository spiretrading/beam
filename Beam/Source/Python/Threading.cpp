#include "Beam/Python/Threading.hpp"
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  struct FromPythonTimer final : VirtualTimer {
    void Start() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualTimer, "start", Start);
    }

    void Cancel() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualTimer, "cancel", Cancel);
    }

    void Wait() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualTimer, "wait", Wait);
    }

    const Publisher<Timer::Result>& GetPublisher() const override {
      PYBIND11_OVERLOAD_PURE_NAME(const Publisher<Timer::Result>&, VirtualTimer,
        "get_publisher", GetPublisher);
    }
  };
}

void Beam::Python::ExportLiveTimer(pybind11::module& module) {
  class_<ToPythonTimer<LiveTimer>, VirtualTimer>(module, "LiveTimer")
    .def(init(
      [] (time_duration interval) {
        return MakeToPythonTimer(std::make_unique<LiveTimer>(interval,
          Ref(*GetTimerThreadPool())));
      }));
}

void Beam::Python::ExportThreading(pybind11::module& module) {
  auto submodule = module.def_submodule("threading");
  ExportTimer(submodule);
  ExportLiveTimer(submodule);
  ExportTriggerTimer(submodule);

#if 0 // TODO exceptions
  ExportException<TimeoutException, std::runtime_error>("TimeoutException")
    .def(init<>())
    .def(init<const string&>());
#endif
}

void Beam::Python::ExportTimer(pybind11::module& module) {
  auto outer = class_<VirtualTimer, FromPythonTimer>(module, "Timer")
    .def("start", &VirtualTimer::Start)
    .def("cancel", &VirtualTimer::Cancel)
    .def("wait", &VirtualTimer::Wait)
    .def("get_publisher", &VirtualTimer::GetPublisher,
      return_value_policy::reference_internal);
  enum_<Timer::Result::Type>(outer, "Result")
    .value("NONE", Timer::Result::NONE)
    .value("EXPIRED", Timer::Result::EXPIRED)
    .value("CANCELED", Timer::Result::CANCELED)
    .value("FAIL", Timer::Result::FAIL);
// TODO, Queues  ExportQueueSuite<Timer::Result>("TimerResult");
}

void Beam::Python::ExportTriggerTimer(pybind11::module& module) {
  class_<ToPythonTimer<TriggerTimer>, VirtualTimer>(module, "TriggerTimer")
    .def("__init__",
      [] {
        return MakeToPythonTimer(std::make_unique<TriggerTimer>());
      })
    .def("trigger",
      [] (ToPythonTimer<TriggerTimer>& self) {
        self.GetTimer().Trigger();
      })
    .def("fail",
      [] (ToPythonTimer<TriggerTimer>& self) {
        self.GetTimer().Fail();
      });
}
