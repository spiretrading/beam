#ifndef BEAM_REGISTRY_SERVICE_HPP
#define BEAM_REGISTRY_SERVICE_HPP
#include <string>

namespace Beam::RegistryService {
  class FileSystemRegistryDataStore;
  class LocalRegistryDataStore;
  template<typename B> class RegistryClient;
  class RegistryClientBox;
  class RegistryDataStore;
  class RegistryDataStoreException;
  struct RegistryEntry;
  template<typename C, typename D> class RegistryServlet;
  class RegistrySession;

  /** Standard name for the registry service. */
  inline const auto SERVICE_NAME = std::string("registry_service");
}

#endif
