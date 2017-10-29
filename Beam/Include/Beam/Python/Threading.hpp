#ifndef BEAM_PYTHON_THREADING_HPP
#define BEAM_PYTHON_THREADING_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the LiveTimer class.
  void ExportLiveTimer();

  //! Exports the Threading namespace.
  void ExportThreading();

  //! Exports the Timer class.
  void ExportTimer();

  //! Exports the TriggerTimer class.
  void ExportTriggerTimer();
}
}

#endif
