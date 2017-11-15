#include "Beam/Python/Threading.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/Python/UniquePtr.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/TimeoutException.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  struct FromPythonTimer : VirtualTimer, wrapper<VirtualTimer> {
    virtual void Start() override final {
      get_override("start")();
    }

    virtual void Cancel() override final {
      get_override("cancel")();
    }

    virtual void Wait() override final {
      get_override("wait")();
    }

    virtual const Publisher<Timer::Result>&
        GetPublisher() const override final {
      return *static_cast<const Publisher<Timer::Result>*>(
        get_override("get_publisher")());
    }
  };

  auto BuildLiveTimer(time_duration interval) {
    return MakeToPythonTimer(std::make_unique<LiveTimer>(interval,
      Ref(*GetTimerThreadPool())));
  }

  auto BuildTriggerTimer() {
    return MakeToPythonTimer(std::make_unique<TriggerTimer>());
  }

  void WrapperTimerTrigger(ToPythonTimer<TriggerTimer>& timer) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    timer.GetTimer().Trigger();
  }

  void WrapperTimerFail(ToPythonTimer<TriggerTimer>& timer) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    timer.GetTimer().Fail();
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(FromPythonTimer);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Publisher<Timer::Result>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualTimer);

void Beam::Python::ExportLiveTimer() {
  class_<ToPythonTimer<LiveTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "LiveTimer", no_init)
    .def("__init__", make_constructor(&BuildLiveTimer));
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
  ExportException<TimeoutException, std::runtime_error>("TimeoutException")
    .def(init<>())
    .def(init<const string&>());
}

void Beam::Python::ExportTimer() {
  {
    scope outer = class_<FromPythonTimer, std::shared_ptr<FromPythonTimer>,
      boost::noncopyable>("Timer")
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
  register_ptr_to_python<std::shared_ptr<VirtualTimer>>();
  implicitly_convertible<std::shared_ptr<FromPythonTimer>,
    std::shared_ptr<VirtualTimer>>();
  ExportUniquePtr<VirtualTimer>();
  ExportEnum<Timer::Result>();
  ExportPublisher<Timer::Result>("TimerResultPublisher");
}

void Beam::Python::ExportTriggerTimer() {
  class_<ToPythonTimer<TriggerTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "TriggerTimer", no_init)
    .def("__init__", make_constructor(&BuildTriggerTimer))
    .def("trigger", &WrapperTimerTrigger)
    .def("fail", &WrapperTimerFail);
}
