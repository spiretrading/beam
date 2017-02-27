#ifndef BEAM_PYTHONROUTINES_HPP
#define BEAM_PYTHONROUTINES_HPP
#include <boost/python.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  void EvalSetResult(Routines::Eval<T>& eval, const T& value) {
    eval.SetResult(value);
  }

  template<typename T>
  std::shared_ptr<Routines::Eval<T>> MakeEval() {
    return std::make_shared<Routines::Eval<T>>();
  }

  template<typename T>
  std::shared_ptr<Routines::Eval<T>> AsyncGetEval(Routines::Async<T>& async) {
    return std::make_shared<Routines::Eval<T>>(async.GetEval());
  }
}

  //! Exports the Eval class.
  template<typename T>
  void ExportEval(const char* name) {
    boost::python::class_<Routines::Eval<T>, std::shared_ptr<Routines::Eval<T>>,
      boost::noncopyable, boost::python::bases<Routines::BaseEval>>(
      name, boost::python::no_init)
      .def("__init__", boost::python::make_constructor(&Details::MakeEval<T>))
      .def("is_empty", &Routines::Eval<T>::IsEmpty)
      .def("set_result", &Details::EvalSetResult<T>);
  }

  //! Exports the Async class.
  template<typename T>
  void ExportAsync(const char* name) {
    boost::python::class_<Routines::Async<T>, boost::noncopyable,
      boost::python::bases<Routines::BaseAsync>>(name, boost::python::init<>())
      .def("get_eval", &Details::AsyncGetEval<T>)
      .def("get", BlockingFunction(&Routines::Async<T>::Get,
        boost::python::return_value_policy<
        boost::python::copy_non_const_reference>()))
      .def("get_exception", &Routines::Async<T>::GetException,
        boost::python::return_value_policy<
        boost::python::copy_const_reference>())
      .add_property("state", &Routines::Async<T>::GetState)
      .def("reset", &Routines::Async<T>::Reset);
  }

  //! Exports the BaseAsync class.
  void ExportBaseAsync();

  //! Exports the BaseEval class.
  void ExportBaseEval();

  //! Exports an Async using native Python objects.
  void ExportPythonAsync();

  //! Exports an Eval using native Python objects.
  void ExportPythonEval();

  //! Exports the RoutineHandler class.
  void ExportRoutineHandler();

  //! Exports the RoutineHandlerGroup class.
  void ExportRoutineHandlerGroup();

  //! Exports the Routines namespace.
  void ExportRoutines();
}
}

#endif
