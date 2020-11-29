#ifndef BEAM_PYTHON_THREADING_HPP
#define BEAM_PYTHON_THREADING_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/Threading/TimerBox.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported TimerBox. */
  BEAM_EXPORT_DLL pybind11::class_<Threading::TimerBox>& GetExportedTimerBox();

  /**
   * Exports the ConditionVariable class.
   * @param module The module to export to.
   */
  void ExportConditionVariable(pybind11::module& module);

  /**
   * Exports the LiveTimer class.
   * @param module The module to export to.
   */
  void ExportLiveTimer(pybind11::module& module);

  /**
   * Exports the Mutex class.
   * @param module The module to export to.
   */
  void ExportMutex(pybind11::module& module);

  /**
   * Exports the RecursiveMutex class.
   * @param module The module to export to.
   */
  void ExportRecursiveMutex(pybind11::module& module);

  /**
   * Exports the Threading namespace.
   * @param module The module to export to.
   */
  void ExportThreading(pybind11::module& module);

  /**
   * Exports the TriggerTimer class.
   * @param module The module to export to.
   */
  void ExportTriggerTimer(pybind11::module& module);

  /**
   * Exports a Timer class.
   * @param <Timer> The type of Timer to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported Timer.
   */
  template<typename Timer>
  auto ExportTimer(pybind11::module& module, const std::string& name) {
    auto timer = pybind11::class_<Timer, std::shared_ptr<Timer>>(module,
      name.c_str()).
      def("start", &Timer::Start).
      def("cancel", &Timer::Cancel).
      def("wait", &Timer::Wait).
      def("get_publisher", &Timer::GetPublisher,
        pybind11::return_value_policy::reference_internal);
    if constexpr(!std::is_same_v<Timer, Threading::TimerBox>) {
      pybind11::implicitly_convertible<Timer, Threading::TimerBox>();
      GetExportedTimerBox().def(pybind11::init<std::shared_ptr<Timer>>());
    } else {
      pybind11::enum_<Threading::Timer::Result::Type>(timer, "Result").
        value("NONE", Threading::Timer::Result::NONE).
        value("EXPIRED", Threading::Timer::Result::EXPIRED).
        value("CANCELED", Threading::Timer::Result::CANCELED).
        value("FAIL", Threading::Timer::Result::FAIL);
    }
    return timer;
  }
}

#endif
