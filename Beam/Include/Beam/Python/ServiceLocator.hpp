#ifndef BEAM_PYTHONSERVICELOCATOR_HPP
#define BEAM_PYTHONSERVICELOCATOR_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the ApplicationServiceLocatorClient class.
  void ExportApplicationServiceLocatorClient();

  //! Exports the DirectoryEntry struct.
  void ExportDirectoryEntry();

  //! Exports the Permissions enum.
  void ExportPermissions();

  //! Exports the ServiceEntry class.
  void ExportServiceEntry();

  //! Exports the ServiceLocator namespace.
  void ExportServiceLocator();

  //! Exports the ServiceLocatorClient class.
  void ExportServiceLocatorClient();

  //! Exports the ServiceLocatorTestEnvironment class.
  void ExportServiceLocatorTestEnvironment();
}
}

#endif
