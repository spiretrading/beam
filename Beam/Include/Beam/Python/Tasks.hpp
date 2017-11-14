#ifndef BEAM_PYTHONTASKS_HPP
#define BEAM_PYTHONTASKS_HPP
#include <boost/python.hpp>
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Tasks/ReactorTask.hpp"

namespace Beam {
namespace Python {

  //! Exports the AggregateTask class.
  void ExportAggregateTask();

  //! Exports the BasicTask class.
  void ExportBasicTask();

  //! Exports the ChainedTask class.
  void ExportChainedTask();

  //! Exports the IdleTask class.
  void ExportIdleTask();

  //! Exports the PythonPackagedTask class.
  void ExportPythonPackagedTask();

  //! Exports the ReactorMonitorTask class.
  void ExportReactorMonitorTask();

  //! Exports the ReactorTask class.
  void ExportReactorTask();

  //! Exports the SpawnTask class.
  void ExportSpawnTask();

  //! Exports the Task class.
  void ExportTask();

  //! Exports the TaskFactory class.
  void ExportTaskFactory();

  //! Exports the Tasks namespace.
  void ExportTasks();

  //! Exports the UntilTask class.
  void ExportUntilTask();

  //! Exports the WhenTask class.
  void ExportWhenTask();

  //! Exports a TypedReactorTaskProperty.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportTypedReactorTaskProperty(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<T,
      boost::python::bases<Tasks::VirtualReactorProperty>>(name,
      boost::python::init<
      const std::string&, std::shared_ptr<
        Reactors::Reactor<typename T::Type>>>())
      .def("__copy__", &MakeCopy<T>)
      .def("__deepcopy__", &MakeDeepCopy<T>);
    boost::python::implicitly_convertible<T, Tasks::ReactorProperty>();
  }
}
}

#endif
