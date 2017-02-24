#ifndef BEAM_PYTHONROUTINES_HPP
#define BEAM_PYTHONROUTINES_HPP
#include "Beam/Python/Python.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {
namespace Python {

  //! Exports the Async class.
  template<typename T>
  void ExportAsync(const char* name) {
    boost::python::class_<Routines::Async<T>, boost::noncopyable,
      boost::python::bases<Routines::BaseAsync>>(name)
      .def("get_eval", &Async<T>::GetEval)
      .def("get", BlockingFunction(&Routines::Async<T>::Get,
        boost::python::return_value_policy<
        boost::python::copy_non_const_reference>()))
      .def("get_exception", &Async<T>::GetException)
      .add_property("state", &Routines::Async<T>::GetState)
      .def("reset", &Routines::Async<T>::Reset);
  }

  //! Exports the RoutineHandler class.
  void ExportRoutineHandler();

  //! Exports the RoutineHandlerGroup class.
  void ExportRoutineHandlerGroup();

  //! Exports the Routines namespace.
  void ExportRoutines();
}
}

#endif
