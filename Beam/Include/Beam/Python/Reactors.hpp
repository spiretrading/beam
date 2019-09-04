#ifndef BEAM_PYTHON_REACTORS_HPP
#define BEAM_PYTHON_REACTORS_HPP
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam::Python {

  //! Exports the AlarmReactor class.
  void ExportAlarmReactor();

  //! Exports the CurrentTimeReactor.
  void ExportCurrentTimeReactor();

  //! Exports the Publisher Reactor.
  void ExportPublisherReactor();

  //! Exports the QueueReactor class.
  void ExportQueueReactor();

  //! Exports the QueryReactor.
  void ExportQueryReactor();

  //! Exports the Reactors namespace.
  void ExportReactors();

  //! Exports the TimerReactor class.
  void ExportTimerReactor();

  //! Exports the Reactor class template.
  /*!
    \param name The name of the class.
  */
  template<typename T>
  void ExportReactor(const char* name) {
    using Reactor = Aspen::Shared<T>;
    auto typeId = boost::python::type_id<Reactor>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Reactor, boost::noncopyable>(name,
        boost::python::no_init)
      .def("commit", &Reactor::commit)
      .def("eval", &Reactor::eval, boost::python::return_value_policy<
        boost::python::copy_const_reference>());
  }
}

#endif
