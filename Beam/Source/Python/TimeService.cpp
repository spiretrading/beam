#include "Beam/Python/TimeService.hpp"
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/Threading.hpp"
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
#include "Beam/TimeServiceTests/TimeServiceTestEnvironmentException.hpp"

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
  struct FromPythonTimerClient : VirtualTimeClient, wrapper<VirtualTimeClient> {
    virtual boost::posix_time::ptime GetTime() override final {
      return get_override("get_time")();
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  template<typename ClientType>
  class ToPythonTimeClient : public VirtualTimeClient {
    public:
      ToPythonTimeClient(std::unique_ptr<ClientType> client)
          : m_client{std::move(client)} {}

      virtual ~ToPythonTimeClient() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_client.reset();
      }

      virtual boost::posix_time::ptime GetTime() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        return m_client->GetTime();
      }

      virtual void Open() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_client->Open();
      }

      virtual void Close() override final {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock{gil};
        m_client->Close();
      }

      std::unique_ptr<ClientType> m_client;
  };

  ToPythonTimeClient<LiveNtpTimeClient>* BuildNtpTimeClient(
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
    return new ToPythonTimeClient<LiveNtpTimeClient>{std::move(timeClient)};
  }

  ToPythonTimeClient<FixedTimeClient>* BuildFixedTimeClient() {
    return new ToPythonTimeClient<FixedTimeClient>{
      std::make_unique<FixedTimeClient>()};
  }

  ToPythonTimeClient<FixedTimeClient>* BuildFixedTimeClient(ptime time) {
    return new ToPythonTimeClient<FixedTimeClient>{
      std::make_unique<FixedTimeClient>(time)};
  }

  ToPythonTimeClient<IncrementalTimeClient>* BuildIncrementalTimeClient() {
    return new ToPythonTimeClient<IncrementalTimeClient>{
      std::make_unique<IncrementalTimeClient>()};
  }

  ToPythonTimeClient<IncrementalTimeClient>* BuildIncrementalTimeClient(
      ptime initialTime, time_duration increment) {
    return new ToPythonTimeClient<IncrementalTimeClient>{
      std::make_unique<IncrementalTimeClient>(initialTime, increment)};
  }

  ToPythonTimeClient<LocalTimeClient>* BuildLocalTimeClient() {
    return new ToPythonTimeClient<LocalTimeClient>{
      std::make_unique<LocalTimeClient>()};
  }

  ToPythonTimeClient<TestTimeClient>* BuildTestTimeClient(
      std::shared_ptr<TimeServiceTestEnvironment> environment) {
    return new ToPythonTimeClient<TestTimeClient>{
      std::make_unique<TestTimeClient>(Ref(*environment))};
  }

  auto BuildTestTimer(time_duration expiry,
      std::shared_ptr<TimeServiceTestEnvironment> environment) {
    return MakeToPythonTimer(std::make_unique<TestTimer>(expiry,
      Ref(*environment))).release();
  }

  void FixedTimeClientSetTime(ToPythonTimeClient<FixedTimeClient>& client,
      ptime time) {
    client.m_client->SetTime(time);
  }
}

void Beam::Python::ExportTzDatabase() {
  class_<tz_database>("TimeZoneDatabase", no_init);
}

void Beam::Python::ExportFixedTimeClient() {
  class_<ToPythonTimeClient<FixedTimeClient>, boost::noncopyable,
      bases<VirtualTimeClient>>("FixedTimeClient", no_init)
    .def("__init__", make_constructor(
      static_cast<ToPythonTimeClient<FixedTimeClient>* (*)()>(
      &BuildFixedTimeClient)))
    .def("__init__", make_constructor(
      static_cast<ToPythonTimeClient<FixedTimeClient>* (*)(ptime)>(
      &BuildFixedTimeClient)))
    .def("set_time", &FixedTimeClientSetTime);
}

void Beam::Python::ExportIncrementalTimeClient() {
  class_<ToPythonTimeClient<IncrementalTimeClient>, boost::noncopyable,
      bases<VirtualTimeClient>>("IncrementalTimeClient", no_init)
    .def("__init__", make_constructor(
      static_cast<ToPythonTimeClient<IncrementalTimeClient>* (*)()>(
      &BuildIncrementalTimeClient)))
    .def("__init__", make_constructor(
      static_cast<ToPythonTimeClient<IncrementalTimeClient>* (*)(ptime,
      time_duration)>(&BuildIncrementalTimeClient)));
}

void Beam::Python::ExportLocalTimeClient() {
  class_<ToPythonTimeClient<LocalTimeClient>, boost::noncopyable,
      bases<VirtualTimeClient>>("LocalTimeClient", no_init)
    .def("__init__", make_constructor(&BuildLocalTimeClient));
}

void Beam::Python::ExportNtpTimeClient() {
  class_<ToPythonTimeClient<LiveNtpTimeClient>, boost::noncopyable,
    bases<VirtualTimeClient>>("NtpTimeClient", no_init)
    .def("__init__", make_constructor(&BuildNtpTimeClient));
}

void Beam::Python::ExportTestTimeClient() {
  class_<ToPythonTimeClient<TestTimeClient>, boost::noncopyable,
    bases<VirtualTimeClient>>("TestTimeClient", no_init)
    .def("__init__", make_constructor(&BuildTestTimeClient));
}

void Beam::Python::ExportTestTimer() {
  class_<ToPythonTimer<TestTimer>, boost::noncopyable, bases<VirtualTimer>>(
      "TestTimer", no_init)
    .def("__init__", make_constructor(&BuildTestTimer));
}

void Beam::Python::ExportTimeClient() {
  class_<VirtualTimeClient, boost::noncopyable>("TimeClient", no_init)
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
    ExportException<TimeServiceTestEnvironmentException, std::runtime_error>(
      "TimeServiceTestEnvironmentException")
      .def(init<>())
      .def(init<const string&>());
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
