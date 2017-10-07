#ifndef BEAM_PYTHONTASKS_HPP
#define BEAM_PYTHONTASKS_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the AggregateTask class.
  void ExportAggregateTask();

  //! Exports the BasicTask class.
  void ExportBasicTask();

  //! Exports the IdleTask class.
  void ExportIdleTask();

  //! Exports the ReactorMonitorTask class.
  void ExportReactorMonitorTask();

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
}
}

#endif
