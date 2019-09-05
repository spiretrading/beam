#ifndef BEAM_PYTHON_COLLECTIONS_HPP
#define BEAM_PYTHON_COLLECTIONS_HPP
#include <pybind11/pybind11.h>
#include "Beam/Collections/View.hpp"

namespace Beam::Python {

  /**
   * Exports the View template.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<typename T>
  void ExportView(pybind11::module& module, const char* name) {
    pybind11::class_<View<T>>(module, name)
      .def("empty", &View<T>::empty)
      .def("front", static_cast<const T& (View<T>::*)() const>(&View<T>::front))
      .def("back", static_cast<const T& (View<T>::*)() const>(&View<T>::back))
      .def("__iter__",
        [] (const View<T>& view) {
          return pybind11::make_iterator(view.begin(), view.end());
        }, pybind11::keep_alive<0, 1>());
  }
}

#endif
