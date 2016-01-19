#include "Beam/Python/TimeService.hpp"
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeService/ToLocalTime.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::local_time;
using namespace boost::python;
using namespace std;

namespace {
  VirtualTimeClient* BuildClient(
      VirtualServiceLocatorClient* serviceLocatorClient) {
    std::unique_ptr<LiveNtpTimeClient> timeClient;
    try {
      auto timeServices = serviceLocatorClient->Locate(
        TimeService::SERVICE_NAME);
      if(timeServices.empty()) {
        BOOST_THROW_EXCEPTION(ConnectException("No time services available."));
      }
      auto& timeService = timeServices.front();
      auto ntpPool = FromString<vector<IpAddress>>(get<string>(
        timeService.GetProperties().At("addresses")));
      timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(*GetSocketThreadPool()),
        Ref(*GetTimerThreadPool()));
    } catch(const  std::exception&) {
      BOOST_THROW_EXCEPTION(ConnectException(
        "Unable to initialize NTP time client."));
    }
    return new WrapperTimeClient<std::unique_ptr<LiveNtpTimeClient>>(
      std::move(timeClient));
  }
}

void Beam::Python::ExportTzDatabase() {
  class_<tz_database>("TimeZoneDatabase", no_init);
}

void Beam::Python::ExportTimeClient() {
  class_<VirtualTimeClient, boost::noncopyable>(
    "TimeClient", no_init)
    .def("__init__", make_constructor(&BuildClient))
    .def("get_time", &VirtualTimeClient::GetTime)
    .def("open", BlockingFunction(&VirtualTimeClient::Open))
    .def("close", BlockingFunction(&VirtualTimeClient::Close));
}

void Beam::Python::ExportTimeService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".time_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("time_service") = nestedModule;
  scope parent = nestedModule;
  def("adjust_date_time", &AdjustDateTime);
  ExportTzDatabase();
  ExportTimeClient();
}
