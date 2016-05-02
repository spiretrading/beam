#ifndef BEAM_PYTHONUIDSERVICE_HPP
#define BEAM_PYTHONUIDSERVICE_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the UidClient class.
  void ExportUidClient();

  //! Exports the UidService namespace.
  void ExportUidService();

  //! Exports the UidService::Tests namespace.
  void ExportUidServiceTests();

  //! Exports the UidServiceTestInstance class.
  void ExportUidServiceTestInstance();
}
}

#endif
