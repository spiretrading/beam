#ifndef BEAM_PYTHONTASKS_HPP
#define BEAM_PYTHONTASKS_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the BasicTask class.
  void ExportBasicTask();

  //! Exports the Task class.
  void ExportTask();

  //! Exports the Tasks namespace.
  void ExportTasks();
}
}

#endif
