#include "Beam/Python/TimeService.hpp"
#include <boost/date_time/local_time/local_time.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonTimeClient.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeService/ToLocalTime.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"
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
  struct TrampolineTimeClient final : VirtualTimeClient {
    ptime GetTime() override {
      PYBIND11_OVERLOAD_PURE_NAME(ptime, VirtualTimeClient, "get_time",
        GetTime);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualTimeClient, "open", Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualTimeClient, "close", Close);
    }
  };
}

void Beam::Python::ExportTzDatabase(pybind11::module& module) {
  class_<tz_database>(module, "TimeZoneDatabase");
}

void Beam::Python::ExportFixedTimeClient(pybind11::module& module) {
  class_<ToPythonTimeClient<FixedTimeClient>, VirtualTimeClient>(module,
      "FixedTimeClient")
    .def(init(
      [] {
        return MakeToPythonTimeClient(std::make_unique<FixedTimeClient>());
      }))
    .def(init(
      [] (ptime time) {
        return MakeToPythonTimeClient(std::make_unique<FixedTimeClient>(time));
      }))
    .def("set_time",
      [] (ToPythonTimeClient<FixedTimeClient>& self, ptime time) {
        self.GetClient().SetTime(time);
      });
}

void Beam::Python::ExportIncrementalTimeClient(pybind11::module& module) {
  class_<ToPythonTimeClient<IncrementalTimeClient>, VirtualTimeClient>(module,
      "IncrementalTimeClient")
    .def(init(
      [] {
        return MakeToPythonTimeClient(
          std::make_unique<IncrementalTimeClient>());
      }))
    .def(init(
      [] (ptime time, time_duration increment) {
        return MakeToPythonTimeClient(
          std::make_unique<IncrementalTimeClient>(time, increment));
      }));
}

void Beam::Python::ExportLocalTimeClient(pybind11::module& module) {
  class_<ToPythonTimeClient<LocalTimeClient>, VirtualTimeClient>(module,
      "LocalTimeClient")
    .def(init(
      [] {
        return MakeToPythonTimeClient(std::make_unique<LocalTimeClient>());
      }));
}

void Beam::Python::ExportNtpTimeClient(pybind11::module& module) {
  class_<ToPythonTimeClient<LiveNtpTimeClient>, VirtualTimeClient>(module,
      "NtpTimeClient")
    .def(init(
      [] (VirtualServiceLocatorClient* serviceLocatorClient) {
        auto timeClient =
          [&] {
            try {
              auto timeServices = serviceLocatorClient->Locate(
                TimeService::SERVICE_NAME);
              if(timeServices.empty()) {
                throw ConnectException("No time services available.");
              }
              auto& timeService = timeServices.front();
              auto ntpPool = FromString<std::vector<IpAddress>>(
                get<std::string>(timeService.GetProperties().At("addresses")));
              return MakeLiveNtpTimeClient(ntpPool, Ref(*GetSocketThreadPool()),
                Ref(*GetTimerThreadPool()));
            } catch(const std::exception&) {
              throw ConnectException("Unable to initialize NTP time client.");
            }
          }();
        return MakeToPythonTimeClient(std::move(timeClient)).release();
      }));
}

void Beam::Python::ExportTestTimeClient(pybind11::module& module) {
  class_<ToPythonTimeClient<TestTimeClient>, VirtualTimeClient>(module,
      "TestTimeClient")
    .def(init(
      [] (TimeServiceTestEnvironment& environment) {
        return MakeToPythonTimeClient(std::make_unique<TestTimeClient>(
          Ref(environment)));
      }));
}

void Beam::Python::ExportTestTimer(pybind11::module& module) {
  class_<ToPythonTimer<TestTimer>, VirtualTimer,
      std::shared_ptr<ToPythonTimer<TestTimer>>>(module, "TestTimer")
    .def(init(
      [] (time_duration expiry, TimeServiceTestEnvironment& environment) {
        return MakeToPythonTimer(std::make_unique<TestTimer>(expiry,
          Ref(environment)));
      }));
}

void Beam::Python::ExportTimeClient(pybind11::module& module) {
  class_<VirtualTimeClient, TrampolineTimeClient>(module, "TimeClient")
    .def("get_time", &VirtualTimeClient::GetTime)
    .def("open", &VirtualTimeClient::Open)
    .def("close", &VirtualTimeClient::Close);
}

void Beam::Python::ExportTimeService(pybind11::module& module) {
  auto submodule = module.def_submodule("time_service");
  ExportTzDatabase(submodule);
  ExportTimeClient(submodule);
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
    .def(init())
  .def("set_time", &TimeServiceTestEnvironment::SetTime,
      call_guard<gil_scoped_release>())
  .def("advance_time", &TimeServiceTestEnvironment::AdvanceTime,
    call_guard<gil_scoped_release>())
  .def("get_time", &TimeServiceTestEnvironment::GetTime,
    call_guard<gil_scoped_release>())
  .def("open", &TimeServiceTestEnvironment::Open,
    call_guard<gil_scoped_release>())
  .def("close", &TimeServiceTestEnvironment::Close,
    call_guard<gil_scoped_release>());
}
