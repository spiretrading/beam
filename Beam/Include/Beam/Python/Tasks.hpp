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

  //! Exports the Task class.
  void ExportTask();

  //! Exports the TaskFactory class.
  void ExportTaskFactory();

  //! Exports the Tasks namespace.
  void ExportTasks();
}
}

#endif
