#ifndef BEAM_PYTHON_ROUTINES_HPP
#define BEAM_PYTHON_ROUTINES_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam::Python {

  /**
   * Exports the generic Eval class.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_eval(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Eval";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Eval<T>, BaseEval>(module, name.c_str()).
      def(pybind11::init()).
      def_property_readonly("is_empty", &Eval<T>::is_empty).
      def("set", [] (Eval<T>& self, const T& value) {
        self.set(value);
      });
  }

  /**
   * Exports the Eval<void> specialization.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<>
  void export_eval<void>(pybind11::module& module, const std::string& prefix);

  /**
   * Exports the generic Async class.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_async(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Async";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Async<T>, BaseAsync>(module, name.c_str()).
      def(pybind11::init()).
      def("get_eval", &Async<T>::get_eval).
      def("get", &Async<T>::get, pybind11::call_guard<GilRelease>()).
      def_property_readonly("exception", &Async<T>::get_exception).
      def_property_readonly("state", &Async<T>::get_state).
      def("reset", &Async<T>::reset);
  }

  /**
   * Exports the BaseAsync class.
   * @param module The module to export to.
   */
  void export_base_async(pybind11::module& module);

  /**
   * Exports the BaseEval class.
   * @param module The module to export to.
   */
  void export_base_eval(pybind11::module& module);

  /**
   * Exports the RoutineHandler class.
   * @param module The module to export to.
   */
  void export_routine_handler(pybind11::module& module);

  /**
   * Exports the RoutineHandlerGroup class.
   * @param module The module to export to.
   */
  void export_routine_handler_group(pybind11::module& module);

  /** Exports the Routines namespace. */
  void export_routines(pybind11::module& module);
}

#endif
