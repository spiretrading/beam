#include "Beam/Python/Threading.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/TimeoutException.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"
#include "Beam/TimeServiceTests/TestTimeClient.hpp"
#include "Beam/TimeServiceTests/TestTimer.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::TimeService::Tests;
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

  template<typename TimerType>
  class ToPythonTimer : public VirtualTimer {
    public:
      ToPythonTimer(std::unique_ptr<TimerType> timer)
          : m_timer{std::move(timer)} {}

      virtual ~ToPythonTimer() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_timer.reset();
      }

      virtual void Start() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_timer->Start();
      }

      virtual void Cancel() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_timer->Cancel();
      }

      virtual void Wait() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_timer->Wait();
      }

      virtual const Publisher<Timer::Result>&
          GetPublisher() const override final {
        return m_timer->GetPublisher();
      }

      std::unique_ptr<TimerType> m_timer;
  };

  ToPythonTimer<LiveTimer>* BuildLiveTimer(time_duration interval) {
    return new ToPythonTimer<LiveTimer>{std::make_unique<LiveTimer>(interval,
      Ref(*GetTimerThreadPool()))};
  }

  ToPythonTimer<TriggerTimer>* BuildTriggerTimer() {
    return new ToPythonTimer<TriggerTimer>{std::make_unique<TriggerTimer>()};
  }

  void WrapperTimerTrigger(ToPythonTimer<TriggerTimer>& timer) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    timer.m_timer->Trigger();
  }

  void WrapperTimerFail(ToPythonTimer<TriggerTimer>& timer) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    timer.m_timer->Fail();
  }

  ToPythonTimer<TestTimer>* BuildTestTimer(time_duration expiry,
      std::shared_ptr<TimeServiceTestEnvironment> environment) {
    return new ToPythonTimer<TestTimer>{std::make_unique<TestTimer>(expiry,
      Ref(*environment))};
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
    scope outer = class_<FromPythonTimer, boost::noncopyable>("Timer")
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

void Beam::Python::ExportTestTimer() {
  class_<ToPythonTimer<TestTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "TestTimer", no_init)
    .def("__init__", make_constructor(&BuildTestTimer));
}

void Beam::Python::ExportTriggerTimer() {
  class_<ToPythonTimer<TriggerTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "TriggerTimer", no_init)
    .def("__init__", make_constructor(&BuildTriggerTimer))
    .def("trigger", &WrapperTimerTrigger)
    .def("fail", &WrapperTimerFail);
}
