#ifndef BEAM_PYTHONCOLLECTIONS_HPP
#define BEAM_PYTHONCOLLECTIONS_HPP
#include <boost/python.hpp>
#include "Beam/Collections/View.hpp"
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the View template.
  template<typename T>
  void ExportView(const char* name) {
    boost::python::class_<View<T>>(name, boost::python::no_init)
      .def("empty", &View<T>::empty)
      .def("front", static_cast<const T& (View<T>::*)() const>(&View<T>::front),
        boost::python::return_value_policy<
        boost::python::copy_const_reference>())
      .def("back", static_cast<const T& (View<T>::*)() const>(&View<T>::back),
        boost::python::return_value_policy<
        boost::python::copy_const_reference>())
      .def("__iter__", boost::python::iterator<View<T>>());
  }
}
}

#endif
