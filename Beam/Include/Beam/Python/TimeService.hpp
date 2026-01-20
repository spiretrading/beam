#ifndef BEAM_PYTHON_TIME_SERVICE_HPP
#define BEAM_PYTHON_TIME_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/TimeService/TimeClient.hpp"
#include "Beam/TimeService/Timer.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported Timer. */
  BEAM_EXPORT_DLL pybind11::class_<Timer>& get_exported_timer();

  /** Returns the exported TimeClient. */
  BEAM_EXPORT_DLL pybind11::class_<TimeClient>& get_exported_time_client();

  /** Exports the alarm reactor. */
  void export_alarm_reactor(pybind11::module& module);

  /** Exports the current time reactor. */
  void export_current_time_reactor(pybind11::module& module);

  /** Exports the FixedTimeClient class. */
  void export_fixed_time_client(pybind11::module& module);

  /** Exports the LiveTimer class. */
  void export_live_timer(pybind11::module& module);

  /** Exports the LocalTimeClient class. */
  void export_local_time_client(pybind11::module& module);

  /** Exports the LiveNtpTimeClient class. */
  void export_ntp_time_client(pybind11::module& module);

  /** Exports the TestTimeClient class. */
  void export_test_time_client(pybind11::module& module);

  /** Exports the TestTimer class. */
  void export_test_timer(pybind11::module& module);

  /** Exports a TimeClient class. */
  template<IsTimeClient T>
  auto export_time_client(pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<T>(module, name.data()).
      def("get_time", &T::get_time).
      def("close", &T::close);
    if constexpr(!std::is_same_v<T, TimeClient>) {
      get_exported_time_client().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<T, TimeClient>();
    }
    return client;
  }

  /** Exports the TimeService namespace. */
  void export_time_service(pybind11::module& module);

  /** Exports the TimeServiceTestEnvironment class. */
  void export_time_service_test_environment(pybind11::module& module);

  /** Exports a Timer class. */
  template<IsTimer T>
  auto export_timer(pybind11::module& module, std::string_view name) {
    auto timer = pybind11::class_<T, std::shared_ptr<T>>(module, name.data()).
      def("start", &T::start).
      def("cancel", &T::cancel).
      def("wait", &T::wait).
      def("get_publisher", &T::get_publisher,
        pybind11::return_value_policy::reference_internal);
    if constexpr(!std::is_same_v<T, Timer>) {
      get_exported_timer().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<T, Timer>();
    } else {
      pybind11::enum_<Beam::Timer::Result::Type>(timer, "Result").
        value("NONE", Beam::Timer::Result::NONE).
        value("EXPIRED", Beam::Timer::Result::EXPIRED).
        value("CANCELED", Beam::Timer::Result::CANCELED).
        value("FAIL", Beam::Timer::Result::FAIL);
    }
    return timer;
  }

  /** Exports the timer reactor. */
  void export_timer_reactor(pybind11::module& module);

  /** Exports the TriggerTimer class. */
  void export_trigger_timer(pybind11::module& module);

  /** Exports the tz_database class. */
  void export_tz_database(pybind11::module& module);
}

#endif
