#include "Beam/Python/TimeService.hpp"
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/Threading/VirtualTimer.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeService/ToLocalTime.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"
#include "Beam/TimeServiceTests/TestTimeClient.hpp"
#include "Beam/TimeServiceTests/TestTimer.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::TimeService::Tests;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  struct VirtualTimeClientWrapper : VirtualTimeClient,
      wrapper<VirtualTimeClient> {
    virtual boost::posix_time::ptime GetTime() override {
      return this->get_override("get_time")();
    }

    virtual void Open() override {
      this->get_override("open")();
    }

    virtual void Close() override {
      this->get_override("close")();
    }
  };

  VirtualTimeClient* BuildNtpTimeClient(
      VirtualServiceLocatorClient* serviceLocatorClient) {
    auto timeClient =
      [&] {
        try {
          auto timeServices = serviceLocatorClient->Locate(
            TimeService::SERVICE_NAME);
          if(timeServices.empty()) {
            BOOST_THROW_EXCEPTION(ConnectException{
              "No time services available."});
          }
          auto& timeService = timeServices.front();
          auto ntpPool = FromString<vector<IpAddress>>(get<string>(
            timeService.GetProperties().At("addresses")));
          return MakeLiveNtpTimeClient(ntpPool, Ref(*GetSocketThreadPool()),
            Ref(*GetTimerThreadPool()));
        } catch(const std::exception&) {
          BOOST_THROW_EXCEPTION(ConnectException{
            "Unable to initialize NTP time client."});
        }
      }();
    return new WrapperTimeClient<std::unique_ptr<LiveNtpTimeClient>>{
      std::move(timeClient)};
  }

  VirtualTimeClient* BuildFixedTimeClient() {
    return new WrapperTimeClient<FixedTimeClient>{Initialize()};
  }

  VirtualTimeClient* BuildFixedTimeClient(ptime time) {
    return new WrapperTimeClient<FixedTimeClient>{Initialize(time)};
  }

  VirtualTimeClient* BuildIncrementalTimeClient() {
    return new WrapperTimeClient<IncrementalTimeClient>{Initialize()};
  }

  VirtualTimeClient* BuildIncrementalTimeClient(ptime initialTime,
      time_duration increment) {
    return new WrapperTimeClient<IncrementalTimeClient>{Initialize(initialTime,
      increment)};
  }

  VirtualTimeClient* BuildLocalTimeClient() {
    return new WrapperTimeClient<LocalTimeClient>{Initialize()};
  }

  void WrapperFixedTimeClientSetTime(WrapperTimeClient<FixedTimeClient>& client,
      ptime time) {
    client.GetClient().SetTime(time);
  }

  class PythonTestTimer : public WrapperTimer<TestTimer> {
    public:
      PythonTestTimer(time_duration expiry,
          std::shared_ptr<TimeServiceTestEnvironment> environment)
          : WrapperTimer<TestTimer>(Initialize(expiry, Ref(*environment))),
            m_environment{std::move(environment)} {}

      virtual ~PythonTestTimer() override {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        Cancel();
      }

    private:
      std::shared_ptr<TimeServiceTestEnvironment> m_environment;
  };

  class PythonTestTimeClient : public WrapperTimeClient<TestTimeClient> {
    public:
      PythonTestTimeClient(
          std::shared_ptr<TimeServiceTestEnvironment> environment)
          : WrapperTimeClient<TestTimeClient>(Initialize(Ref(*environment))),
            m_environment{std::move(environment)} {}

      virtual ~PythonTestTimeClient() override {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        Close();
      }

    private:
      std::shared_ptr<TimeServiceTestEnvironment> m_environment;
  };

  VirtualTimer* BuildTestTimer(time_duration expiry,
      std::shared_ptr<TimeServiceTestEnvironment> environment) {
    return new PythonTestTimer{expiry, environment};
  }

  VirtualTimeClient* BuildTestTimeClient(
      std::shared_ptr<TimeServiceTestEnvironment> environment) {
    return new PythonTestTimeClient{environment};
  }
}

void Beam::Python::ExportTzDatabase() {
  class_<tz_database>("TimeZoneDatabase", no_init);
}

void Beam::Python::ExportFixedTimeClient() {
  class_<WrapperTimeClient<FixedTimeClient>, boost::noncopyable,
      bases<VirtualTimeClient>>("FixedTimeClient", no_init)
    .def("__init__", make_constructor(
      static_cast<VirtualTimeClient* (*)()>(&BuildFixedTimeClient)))
    .def("__init__", make_constructor(
      static_cast<VirtualTimeClient* (*)(ptime)>(&BuildFixedTimeClient)))
    .def("set_time", &WrapperFixedTimeClientSetTime)
    .def("get_time", &WrapperTimeClient<FixedTimeClient>::GetTime)
    .def("open", BlockingFunction(&WrapperTimeClient<FixedTimeClient>::Open))
    .def("close", BlockingFunction(&WrapperTimeClient<FixedTimeClient>::Close));
}

void Beam::Python::ExportIncrementalTimeClient() {
  class_<WrapperTimeClient<IncrementalTimeClient>, boost::noncopyable,
      bases<VirtualTimeClient>>("IncrementalTimeClient", no_init)
    .def("__init__", make_constructor(
      static_cast<VirtualTimeClient* (*)()>(&BuildIncrementalTimeClient)))
    .def("__init__", make_constructor(
      static_cast<VirtualTimeClient* (*)(ptime, time_duration)>(
      &BuildIncrementalTimeClient)))
    .def("get_time", &WrapperTimeClient<IncrementalTimeClient>::GetTime)
    .def("open", BlockingFunction(
      &WrapperTimeClient<IncrementalTimeClient>::Open))
    .def("close", BlockingFunction(
      &WrapperTimeClient<IncrementalTimeClient>::Close));
}

void Beam::Python::ExportLocalTimeClient() {
  class_<WrapperTimeClient<LocalTimeClient>, boost::noncopyable,
      bases<VirtualTimeClient>>("LocalTimeClient", no_init)
    .def("__init__", make_constructor(&BuildLocalTimeClient))
    .def("get_time", &WrapperTimeClient<LocalTimeClient>::GetTime)
    .def("open", BlockingFunction(&WrapperTimeClient<LocalTimeClient>::Open))
    .def("close", BlockingFunction(&WrapperTimeClient<LocalTimeClient>::Close));
}

void Beam::Python::ExportNtpTimeClient() {
  class_<WrapperTimeClient<std::unique_ptr<LiveNtpTimeClient>>,
      boost::noncopyable, bases<VirtualTimeClient>>("NtpTimeClient", no_init)
    .def("__init__", make_constructor(&BuildNtpTimeClient))
    .def("get_time",
      &WrapperTimeClient<std::unique_ptr<LiveNtpTimeClient>>::GetTime)
    .def("open", BlockingFunction(
      &WrapperTimeClient<std::unique_ptr<LiveNtpTimeClient>>::Open))
    .def("close", BlockingFunction(
      &WrapperTimeClient<std::unique_ptr<LiveNtpTimeClient>>::Close));
}

void Beam::Python::ExportTestTimeClient() {
  class_<PythonTestTimeClient, boost::noncopyable, bases<VirtualTimeClient>>(
      "TestTimeClient", no_init)
    .def("__init__", make_constructor(&BuildTestTimeClient))
    .def("get_time", &PythonTestTimeClient::GetTime)
    .def("open", BlockingFunction<PythonTestTimeClient>(
      &PythonTestTimeClient::Open))
    .def("close", BlockingFunction<PythonTestTimeClient>(
      &PythonTestTimeClient::Close));
}

void Beam::Python::ExportTestTimer() {
  class_<PythonTestTimer, boost::noncopyable, bases<VirtualTimer>>("TestTimer",
      no_init)
    .def("__init__", make_constructor(&BuildTestTimer))
    .def("start", BlockingFunction<PythonTestTimer>(&PythonTestTimer::Start))
    .def("cancel", BlockingFunction<PythonTestTimer>(&PythonTestTimer::Cancel))
    .def("wait", BlockingFunction<PythonTestTimer>(&PythonTestTimer::Wait));
}

void Beam::Python::ExportTimeClient() {
  class_<VirtualTimeClientWrapper, boost::noncopyable>("TimeClient")
    .def("get_time", pure_virtual(&VirtualTimeClient::GetTime))
    .def("open", pure_virtual(&VirtualTimeClient::Open))
    .def("close", pure_virtual(&VirtualTimeClient::Close));
}

void Beam::Python::ExportTimeService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".time_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("time_service") = nestedModule;
  scope parent = nestedModule;
  def("get_utc_offset", &GetUtcOffset);
  def("to_local_time", static_cast<ptime (*)(const ptime&)>(&ToLocalTime));
  def("to_utc_time", static_cast<ptime (*)(const ptime&)>(&ToUtcTime));
  def("to_local_time", static_cast<time_duration (*)(const time_duration&)>(
    &ToLocalTime));
  def("to_utc_time", static_cast<time_duration (*)(const time_duration&)>(
    &ToUtcTime));
  def("adjust_date_time", &AdjustDateTime);
  ExportTzDatabase();
  ExportTimeClient();
  ExportFixedTimeClient();
  ExportIncrementalTimeClient();
  ExportLocalTimeClient();
  ExportNtpTimeClient();
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportTimeServiceTestEnvironment();
    ExportTestTimeClient();
    ExportTestTimer();
  }
}

void Beam::Python::ExportTimeServiceTestEnvironment() {
  class_<TimeServiceTestEnvironment,
      std::shared_ptr<TimeServiceTestEnvironment>, boost::noncopyable>(
      "TimeServiceTestEnvironment", init<>())
    .def("set_time", BlockingFunction(&TimeServiceTestEnvironment::SetTime))
    .def("advance_time", BlockingFunction(
      &TimeServiceTestEnvironment::AdvanceTime))
    .def("get_time", BlockingFunction(&TimeServiceTestEnvironment::GetTime))
    .def("open", BlockingFunction(&TimeServiceTestEnvironment::Open))
    .def("close", BlockingFunction(&TimeServiceTestEnvironment::Close));
}
