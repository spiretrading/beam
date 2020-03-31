#include "Beam/Python/Threading.hpp"
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/TimeoutException.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  struct TrampolineTimer final : VirtualTimer {
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

void Beam::Python::ExportConditionVariable(module& module) {
  class_<ConditionVariable>(module, "ConditionVariable")
    .def(init<>())
    .def("wait",
      [] (ConditionVariable& self, Mutex& m) {
        auto lock = std::unique_lock(m, std::try_to_lock);
        self.wait(lock);
      }, call_guard<GilRelease>())
    .def("notify_one", &ConditionVariable::notify_one, call_guard<GilRelease>())
    .def("notify_all", &ConditionVariable::notify_all,
      call_guard<GilRelease>());
}

void Beam::Python::ExportLiveTimer(module& module) {
  class_<ToPythonTimer<LiveTimer>, VirtualTimer,
      std::shared_ptr<ToPythonTimer<LiveTimer>>>(module, "LiveTimer")
    .def(init(
      [] (time_duration interval) {
        return MakeToPythonTimer(std::make_unique<LiveTimer>(interval,
          Ref(*GetTimerThreadPool())));
      }));
}

void Beam::Python::ExportMutex(module& module) {
  class_<Mutex>(module, "Mutex")
    .def(init<>())
    .def("lock", &Mutex::lock, call_guard<GilRelease>())
    .def("try_lock", &Mutex::try_lock, call_guard<GilRelease>())
    .def("unlock", &Mutex::unlock, call_guard<GilRelease>())
    .def("__enter__",
      [] (object& self) {
        self.attr("lock")();
        return self;
      })
    .def("__exit__",
      [] (object& self) {
        self.attr("unlock")();
      });
}

void Beam::Python::ExportRecursiveMutex(module& module) {
  class_<RecursiveMutex>(module, "RecursiveMutex")
    .def(init<>())
    .def("lock", &RecursiveMutex::lock, call_guard<GilRelease>())
    .def("try_lock", &RecursiveMutex::try_lock, call_guard<GilRelease>())
    .def("unlock", &RecursiveMutex::unlock, call_guard<GilRelease>())
    .def("__enter__",
      [] (object& self) {
        self.attr("lock")();
        return self;
      })
    .def("__exit__",
      [] (object& self) {
        self.attr("unlock")();
      });
}

void Beam::Python::ExportThreading(module& module) {
  auto submodule = module.def_submodule("threading");
  ExportConditionVariable(submodule);
  ExportMutex(submodule);
  ExportRecursiveMutex(submodule);
  ExportTimer(submodule);
  ExportLiveTimer(submodule);
  ExportTriggerTimer(submodule);
  register_exception<TimeoutException>(submodule, "TimeoutException");
}

void Beam::Python::ExportTimer(module& module) {
  auto outer = class_<VirtualTimer, TrampolineTimer,
      std::shared_ptr<VirtualTimer>>(module, "Timer")
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
  ExportQueueSuite<Timer::Result>(module, "TimerResult");
}

void Beam::Python::ExportTriggerTimer(module& module) {
  class_<ToPythonTimer<TriggerTimer>, VirtualTimer,
      std::shared_ptr<ToPythonTimer<TriggerTimer>>>(module, "TriggerTimer")
    .def(init(
      [] {
        return MakeToPythonTimer(std::make_unique<TriggerTimer>());
      }))
    .def("trigger",
      [] (ToPythonTimer<TriggerTimer>& self) {
        self.GetTimer().Trigger();
      })
    .def("fail",
      [] (ToPythonTimer<TriggerTimer>& self) {
        self.GetTimer().Fail();
      });
}
