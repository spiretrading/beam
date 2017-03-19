#include "Beam/Python/Threading.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/Queues.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  struct VirtualTimerWrapper : VirtualTimer, wrapper<VirtualTimer> {
    virtual void Start() override {
      this->get_override("start")();
    }

    virtual void Cancel() override {
      this->get_override("cancel")();
    }

    virtual void Wait() override {
      this->get_override("wait")();
    }

    virtual const Publisher<Timer::Result>& GetPublisher() const override {
      return *static_cast<const Publisher<Timer::Result>*>(
        this->get_override("get_publisher")());
    }
  };

  VirtualTimer* BuildLiveTimer(time_duration interval) {
    return new WrapperTimer<LiveTimer>{
      Initialize(interval, Ref(*GetTimerThreadPool()))};
  }

  VirtualTimer* BuildTriggerTimer() {
    return new WrapperTimer<TriggerTimer>{Initialize()};
  }

  void WrapperTimerTrigger(WrapperTimer<TriggerTimer>& timer) {
    timer.GetTimer().Trigger();
  }

  void WrapperTimerFail(WrapperTimer<TriggerTimer>& timer) {
    timer.GetTimer().Fail();
  }
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile Publisher<Timer::Result>*
      get_pointer(const volatile Publisher<Timer::Result>* p) {
    return p;
  }
}
#endif

void Beam::Python::ExportLiveTimer() {
  class_<WrapperTimer<LiveTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "LiveTimer", no_init)
    .def("__init__", make_constructor(&BuildLiveTimer))
    .def("start", BlockingFunction(&WrapperTimer<LiveTimer>::Start))
    .def("cancel", BlockingFunction(&WrapperTimer<LiveTimer>::Cancel))
    .def("wait", BlockingFunction(&WrapperTimer<LiveTimer>::Wait))
    .def("get_publisher", &WrapperTimer<LiveTimer>::GetPublisher,
      return_internal_reference<>());
}

void Beam::Python::ExportThreading() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".threading");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("threading") = nestedModule;
  scope parent = nestedModule;
  ExportTimer();
  ExportLiveTimer();
  ExportTriggerTimer();
}

void Beam::Python::ExportTimer() {
  {
    scope outer = class_<VirtualTimerWrapper, boost::noncopyable>("Timer")
      .def("start", pure_virtual(&VirtualTimer::Start))
      .def("cancel", pure_virtual(&VirtualTimer::Cancel))
      .def("wait", pure_virtual(&VirtualTimer::Wait))
      .def("get_publisher", pure_virtual(&VirtualTimer::GetPublisher),
        return_internal_reference<>());
    enum_<Timer::Result::Type>("Result")
      .value("NONE", Timer::Result::NONE)
      .value("EXPIRED", Timer::Result::EXPIRED)
      .value("CANCELED", Timer::Result::CANCELED)
      .value("FAIL", Timer::Result::FAIL);
  }
  ExportEnum<Timer::Result>();
  ExportPublisher<Timer::Result>("TimerResultPublisher");
}

void Beam::Python::ExportTriggerTimer() {
  class_<WrapperTimer<TriggerTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "TriggerTimer", no_init)
    .def("__init__", make_constructor(&BuildTriggerTimer))
    .def("trigger", BlockingFunction(&WrapperTimerTrigger))
    .def("fail", BlockingFunction(&WrapperTimerFail))
    .def("start", BlockingFunction(&WrapperTimer<TriggerTimer>::Start))
    .def("cancel", BlockingFunction(&WrapperTimer<TriggerTimer>::Cancel))
    .def("wait", BlockingFunction(&WrapperTimer<TriggerTimer>::Wait))
    .def("get_publisher", &WrapperTimer<TriggerTimer>::GetPublisher,
      return_internal_reference<>());
}
