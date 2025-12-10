#include "Beam/Python/TimeService.hpp"
#include <Aspen/Conversions.hpp>
#include <Aspen/Python/Box.hpp>
#include <Aspen/Python/Reactor.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonTimeClient.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/TimeService/AlarmReactor.hpp"
#include "Beam/TimeService/CurrentTimeReactor.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeService/TimerReactor.hpp"
#include "Beam/TimeService/ToLocalTime.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"
#include "Beam/TimeServiceTests/TestTimeClient.hpp"
#include "Beam/TimeServiceTests/TestTimer.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  auto time_client = std::unique_ptr<class_<TimeClient>>();
  auto timer = std::unique_ptr<class_<Timer>>();
}

class_<TimeClient>& Beam::Python::get_exported_time_client() {
  return *time_client;
}

class_<Timer>& Beam::Python::get_exported_timer() {
  return *timer;
}

void Beam::Python::export_alarm_reactor(pybind11::module& module) {
  module.def("alarm", [] (SharedBox<ptime> expiry) {
    return to_object(alarm_reactor(LocalTimeClient(),
      [] (time_duration duration) {
        return std::make_unique<LiveTimer>(duration);
      }, std::move(expiry)));
  });
  module.def("alarm",
    [] (TimeClient time_client,
        std::function<Timer (time_duration)> timer_factory,
        SharedBox<ptime> expiry) {
      return to_object(alarm_reactor(std::move(time_client),
        [=] (time_duration duration) {
          return timer_factory(duration);
        }, std::move(expiry)));
    });
}

void Beam::Python::export_current_time_reactor(pybind11::module& module) {
  module.def("current_time", [] {
    return to_object(current_time_reactor(LocalTimeClient()));
  });
  module.def("current_time", [] (TimeClient time_client) {
    return to_object(current_time_reactor(std::move(time_client)));
  });
  module.def("current_time", [] (SharedBox<void> pulse) {
    return to_object(current_time_reactor(LocalTimeClient(), std::move(pulse)));
  });
  module.def("current_time",
    [] (TimeClient time_client, SharedBox<void> pulse) {
      return to_object(
        current_time_reactor(std::move(time_client), std::move(pulse)));
    });
}

void Beam::Python::export_fixed_time_client(pybind11::module& module) {
  export_time_client<ToPythonTimeClient<FixedTimeClient>>(
    module, "FixedTimeClient").
    def(pybind11::init()).
    def(pybind11::init<ptime>()).
    def("set", [] (ToPythonTimeClient<FixedTimeClient>& self, ptime time) {
      self.get().set(time);
    });
}

void Beam::Python::export_local_time_client(pybind11::module& module) {
  export_time_client<ToPythonTimeClient<LocalTimeClient>>(
    module, "LocalTimeClient").
    def(pybind11::init());
}

void Beam::Python::export_ntp_time_client(pybind11::module& module) {
  module.def("NtpTimeClient",
    [] (const std::vector<IpAddress>& ntp_pool) {
      return std::make_unique<ToPythonTimeClient<TimeClient>>(
        make_live_ntp_time_client(ntp_pool));
    }, call_guard<gil_scoped_release>());
  module.def("NtpTimeClient",
    [] (const std::vector<IpAddress>& ntp_pool, time_duration sync_period) {
      return std::make_unique<ToPythonTimeClient<TimeClient>>(
        make_live_ntp_time_client(ntp_pool, sync_period));
    }, call_guard<gil_scoped_release>());
  module.def("NtpTimeClient", [] (ServiceLocatorClient service_locator_client) {
    return std::make_unique<ToPythonTimeClient<TimeClient>>(
      make_live_ntp_time_client(service_locator_client));
  }, call_guard<gil_scoped_release>());
}

void Beam::Python::export_live_timer(pybind11::module& module) {
  export_timer<ToPythonTimer<LiveTimer>>(module, "LiveTimer").
    def(pybind11::init<time_duration>());
}

void Beam::Python::export_test_time_client(pybind11::module& module) {
  export_time_client<ToPythonTimeClient<TestTimeClient>>(
    module, "TestTimeClient").
    def(pybind11::init<Ref<TimeServiceTestEnvironment>>(),
      call_guard<gil_scoped_release>());
}

void Beam::Python::export_test_timer(pybind11::module& module) {
  export_timer<ToPythonTimer<TestTimer>>(module, "TestTimer").
    def(pybind11::init<time_duration, Ref<TimeServiceTestEnvironment>>());
}

void Beam::Python::export_time_service(pybind11::module& module) {
  {
    auto aspen = pybind11::module::import("aspen");
    export_box<std::int64_t>(aspen, "Int64");
    export_box<time_duration>(module, "TimeDuration");
    export_box<ptime>(module, "PosixTime");
  }
  export_tz_database(module);
  time_client = std::make_unique<class_<TimeClient>>(
    export_time_client<TimeClient>(module, "TimeClient"));
  timer = std::make_unique<class_<Timer>>(export_timer<Timer>(module, "Timer"));
  export_alarm_reactor(module);
  export_current_time_reactor(module);
  export_fixed_time_client(module);
  export_local_time_client(module);
  export_ntp_time_client(module);
  export_live_timer(module);
  export_timer_reactor(module);
  export_trigger_timer(module);
  module.def("to_local_time", overload_cast<ptime>(&to_local_time));
  module.def("to_utc_time", overload_cast<ptime>(&to_utc_time));
  module.def("to_local_time", overload_cast<time_duration>(&to_local_time));
  module.def("to_utc_time", overload_cast<time_duration>(&to_utc_time));
  module.def("adjust_date_time", &adjust_date_time);
  auto test_module = module.def_submodule("tests");
  export_test_time_client(test_module);
  export_test_timer(test_module);
  export_time_service_test_environment(test_module);
  register_exception<TimeServiceTestEnvironmentException>(
    test_module, "TimeServiceTestEnvironmentException");
}

void Beam::Python::export_time_service_test_environment(
    pybind11::module& module) {
  class_<
    TimeServiceTestEnvironment, std::shared_ptr<TimeServiceTestEnvironment>>(
      module, "TimeServiceTestEnvironment").
    def(pybind11::init(&make_python_shared<TimeServiceTestEnvironment>),
      call_guard<gil_scoped_release>()).
    def(pybind11::init(&make_python_shared<TimeServiceTestEnvironment, ptime>),
      call_guard<gil_scoped_release>()).
    def("set", &TimeServiceTestEnvironment::set,
      call_guard<gil_scoped_release>()).
    def("advance", &TimeServiceTestEnvironment::advance,
      call_guard<gil_scoped_release>()).
    def("get_time", &TimeServiceTestEnvironment::get_time,
      call_guard<gil_scoped_release>()).
    def("close", &TimeServiceTestEnvironment::close,
      call_guard<gil_scoped_release>());
}

void Beam::Python::export_timer_reactor(pybind11::module& module) {
  module.def("timer", [] (SharedBox<time_duration> period) {
    return to_object(timer_reactor<std::int64_t>([] (time_duration duration) {
      return std::make_unique<LiveTimer>(duration);
    }, std::move(period)));
  });
  module.def("timer",
    [] (std::function<std::shared_ptr<Timer> (time_duration)> timer_factory,
        SharedBox<time_duration> period) {
      return to_object(timer_reactor<std::int64_t>(
        std::move(timer_factory), std::move(period)));
    });
}

void Beam::Python::export_trigger_timer(pybind11::module& module) {
  export_timer<ToPythonTimer<TriggerTimer>>(module, "TriggerTimer").
    def(pybind11::init());
}

void Beam::Python::export_tz_database(pybind11::module& module) {
  class_<tz_database>(module, "TimeZoneDatabase");
}
