#ifndef BEAM_PYTHONNETWORK_HPP
#define BEAM_PYTHONNETWORK_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the IpAddress struct.
  void ExportIpAddress();

  //! Exports the Network namespace.
  void ExportNetwork();
}
}

#endif
