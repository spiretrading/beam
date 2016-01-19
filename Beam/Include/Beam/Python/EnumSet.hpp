#ifndef BEAM_PYTHONENUMSET_HPP
#define BEAM_PYTHONENUMSET_HPP
#include <boost/python.hpp>
#include "Beam/Collections/EnumSet.hpp"
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports an EnumSet.
  /*!
    \param name The name to give to the EnumSet.
  */
  template<typename T>
  void ExportEnumSet(const char* name) {
    boost::python::class_<EnumSet<T>>(name, boost::python::init<>())
      .def(boost::python::init<T>())
      .def(boost::python::init<typename T::Type>())
      .def("test", &EnumSet<T>::Test)
      .def("set", &EnumSet<T>::Set)
      .def("unset", &EnumSet<T>::Unset)
      .def(boost::python::self == boost::python::self)
      .def(boost::python::self != boost::python::self)
      .def(boost::python::self & boost::python::self)
      .def(boost::python::self | boost::python::self)
      .def(boost::python::self ^ boost::python::self);
  }
}
}

#endif
