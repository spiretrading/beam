#ifndef BEAM_PYTHONUIDSERVICE_HPP
#define BEAM_PYTHONUIDSERVICE_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the ApplicationUidClient class. 
  void ExportApplicationUidClient();

  //! Exports the UidClient class.
  void ExportUidClient();

  //! Exports the UidService namespace.
  void ExportUidService();

  //! Exports the UidServiceTestEnvironment class.
  void ExportUidServiceTestEnvironment();
}
}

#endif
