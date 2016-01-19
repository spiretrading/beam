#ifndef BEAM_PYTHONTHREADING_HPP
#define BEAM_PYTHONTHREADING_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the LiveTimer class.
  void ExportLiveTimer();

  //! Exports the Threading namespace.
  void ExportThreading();

  //! Exports the Timer class.
  void ExportTimer();
}
}

#endif
