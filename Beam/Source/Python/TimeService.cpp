#include "Beam/Python/TimeService.hpp"
#include <boost/date_time/local_time/local_time.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonTimeClient.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClientBox.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeService/ToLocalTime.hpp"
#include "Beam/TimeServiceTests/TestTimeClient.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"
#include "Beam/TimeServiceTests/TestTimer.hpp"

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
using namespace pybind11;

namespace {
  auto timeClientBox = std::unique_ptr<class_<TimeClientBox>>();
}

class_<TimeClientBox>& Beam::Python::GetExportedTimeClientBox() {
  return *timeClientBox;
}

void Beam::Python::ExportTzDatabase(pybind11::module& module) {
  class_<tz_database>(module, "TimeZoneDatabase");
}

void Beam::Python::ExportFixedTimeClient(pybind11::module& module) {
  ExportTimeClient<ToPythonTimeClient<FixedTimeClient>>(module,
    "FixedTimeClient").
    def(init()).
    def(init<ptime>()).
    def("set_time", [] (ToPythonTimeClient<FixedTimeClient>& self, ptime time) {
      self.GetClient().SetTime(time);
    });
}

void Beam::Python::ExportIncrementalTimeClient(pybind11::module& module) {
  ExportTimeClient<ToPythonTimeClient<IncrementalTimeClient>>(module,
    "IncrementalTimeClient").
    def(init()).
    def(init<ptime, time_duration>());
}

void Beam::Python::ExportLocalTimeClient(pybind11::module& module) {
  ExportTimeClient<ToPythonTimeClient<LocalTimeClient>>(module,
    "LocalTimeClient").
    def(init());
}

void Beam::Python::ExportNtpTimeClient(pybind11::module& module) {
  module.def("NtpTimeClient",
    [] (const std::vector<IpAddress>& ntpPool) {
      return ToPythonTimeClient<TimeClientBox>(MakeLiveNtpTimeClient(ntpPool));
    }, call_guard<GilRelease>());
  module.def("NtpTimeClient",
    [] (const std::vector<IpAddress>& ntpPool, time_duration syncPeriod) {
      return ToPythonTimeClient<TimeClientBox>(
        MakeLiveNtpTimeClient(ntpPool, syncPeriod));
    }, call_guard<GilRelease>());
  module.def("NtpTimeClient",
    [] (ServiceLocatorClientBox serviceLocatorClient) {
      return ToPythonTimeClient<TimeClientBox>(
        MakeLiveNtpTimeClientFromServiceLocator(serviceLocatorClient));
    }, call_guard<GilRelease>());
}

void Beam::Python::ExportTestTimeClient(pybind11::module& module) {
  ExportTimeClient<ToPythonTimeClient<TestTimeClient>>(module,
    "TestTimeClient").
    def(init<Ref<TimeServiceTestEnvironment>>(), call_guard<GilRelease>());
}

void Beam::Python::ExportTestTimer(pybind11::module& module) {
  ExportTimer<ToPythonTimer<TestTimer>>(module, "TestTimer").
    def(init<time_duration, Ref<TimeServiceTestEnvironment>>());
}

void Beam::Python::ExportTimeService(pybind11::module& module) {
  auto submodule = module.def_submodule("time_service");
  ExportTzDatabase(submodule);
  timeClientBox = std::make_unique<class_<TimeClientBox>>(
    ExportTimeClient<TimeClientBox>(submodule, "TimeClient"));
  ExportTimeClient<ToPythonTimeClient<TimeClientBox>>(submodule,
    "TimeClientBox");
  ExportFixedTimeClient(submodule);
  ExportIncrementalTimeClient(submodule);
  ExportLocalTimeClient(submodule);
  ExportNtpTimeClient(submodule);
  submodule.def("to_local_time", static_cast<ptime (*)(const ptime&)>(
    &ToLocalTime));
  submodule.def("to_utc_time", static_cast<ptime (*)(const ptime&)>(
    &ToUtcTime));
  submodule.def("to_local_time", static_cast<time_duration (*)(
    const time_duration&)>(&ToLocalTime));
  submodule.def("to_utc_time", static_cast<time_duration (*)(
    const time_duration&)>(&ToUtcTime));
  submodule.def("adjust_date_time", &AdjustDateTime);
  auto test_module = submodule.def_submodule("tests");
  ExportTestTimeClient(test_module);
  ExportTestTimer(test_module);
  ExportTimeServiceTestEnvironment(test_module);
  register_exception<TimeServiceTestEnvironmentException>(test_module,
    "TimeServiceTestEnvironmentException");
}

void Beam::Python::ExportTimeServiceTestEnvironment(pybind11::module& module) {
  class_<TimeServiceTestEnvironment>(module, "TimeServiceTestEnvironment")
    .def(init(), call_guard<GilRelease>())
    .def(init<ptime>(), call_guard<GilRelease>())
    .def("__del__",
      [] (TimeServiceTestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>())
    .def("set_time", &TimeServiceTestEnvironment::SetTime,
      call_guard<GilRelease>())
    .def("advance_time", &TimeServiceTestEnvironment::AdvanceTime,
      call_guard<GilRelease>())
    .def("get_time", &TimeServiceTestEnvironment::GetTime,
      call_guard<GilRelease>())
    .def("close", &TimeServiceTestEnvironment::Close, call_guard<GilRelease>());
}
