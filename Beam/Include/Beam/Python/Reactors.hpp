#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <Aspen/Python/Reactor.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Reactors/QueueReactor.hpp"

namespace Beam::Python {

  /** Exports the CurrentTimeReactor class. */
  void ExportCurrentTimeReactor(pybind11::module& module);

  /** Exports the QueueReactor class. */
  void ExportQueueReactor(pybind11::module& module);

  /**
   * Exports the generic QueueReactor class.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void ExportQueueReactor(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "QueueReactor";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    Aspen::export_reactor<QueueReactor<T>>(module, name)
      .def(pybind11::init<std::shared_ptr<QueueReader<T>>>());
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<QueueReactor<T>,
        QueueReactor<pybind11::object>>();
    }
  }

  /** Exports the TimerReactor class. */
  void ExportTimerReactor(pybind11::module& module);

  /** Exports the Reactors namespace. */
  void ExportReactors(pybind11::module& module);
}

#endif
