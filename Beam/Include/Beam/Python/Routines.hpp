#ifndef BEAM_PYTHON_ROUTINES_HPP
#define BEAM_PYTHON_ROUTINES_HPP
#include <pybind11/pybind11.h>
#include "Beam/Routines/Async.hpp"

namespace Beam::Python {

  /**
   * Exports the generic Eval class.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void ExportEval(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Eval";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Routines::Eval<T>, Routines::BaseEval>(module,
        name.c_str())
      .def(pybind11::init())
      .def("is_empty", &Routines::Eval<T>::IsEmpty)
      .def("set_result",
        [] (Routines::Eval<T>& self) {
          self.SetResult();
        })
      .def("set_result",
        [] (Routines::Eval<T>& self, const T& value) {
          self.SetResult(value);
        });
  }

  /**
   * Exports the generic Async class.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void ExportAsync(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Async";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Routines::Async<T>, Routines::BaseAsync>(module,
        name.c_str())
      .def(pybind11::init())
      .def("get_eval", &Routines::Async<T>::GetEval)
      .def("get", &Routines::Async<T>::Get,
        pybind11::call_guard<pybind11::gil_scoped_release>())
      .def("get_exception", &Routines::Async<T>::GetException)
      .def_property_readonly("state", &Routines::Async<T>::GetState)
      .def("reset", &Routines::Async<T>::Reset);
  }

  /**
   * Exports the BaseAsync class.
   * @param module The module to export to.
   */
  void ExportBaseAsync(pybind11::module& module);

  /**
   * Exports the BaseEval class.
   * @param module The module to export to.
   */
  void ExportBaseEval(pybind11::module& module);

  /**
   * Exports the RoutineHandler class.
   * @param module The module to export to.
   */
  void ExportRoutineHandler(pybind11::module& module);

  /**
   * Exports the RoutineHandlerGroup class.
   * @param module The module to export to.
   */
  void ExportRoutineHandlerGroup(pybind11::module& module);

  /**
   * Exports the Routines namespace.
   */
  void ExportRoutines(pybind11::module& module);
}

#endif
