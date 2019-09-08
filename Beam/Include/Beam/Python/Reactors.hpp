#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <Aspen/Python/Reactor.hpp>
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the AlarmReactor.
   * @param module The module to export to.
   */
  void ExportAlarmReactor(pybind11::module& module);

  /**
   * Exports the CurrentTimeReactor.
   * @param module The module to export to.
   */
  void ExportCurrentTimeReactor(pybind11::module& module);

  /**
   * Exports the PublisherReactor.
   * @param module The module to export to.
   */
  void ExportPublisherReactor(pybind11::module& module);

  /**
   * Exports the QueryReactor.
   * @param module The module to export to.
   */
  void ExportQueryReactor(pybind11::module& module);

  /**
   * Exports the QueueReactor.
   * @param module The module to export to.
   */
  void ExportQueueReactor(pybind11::module& module);

  /**
   * Exports the TimerReactor.
   * @param module The module to export to.
   */
  void ExportTimerReactor(pybind11::module& module);

  /**
   * Exports the Reactors namespace.
   * @param module The module to export to.
   */
  void ExportReactors(pybind11::module& module);
}

#endif
