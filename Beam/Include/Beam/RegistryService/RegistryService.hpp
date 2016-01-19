#ifndef BEAM_REGISTRYSERVICE_HPP
#define BEAM_REGISTRYSERVICE_HPP
#include <string>

namespace Beam {
namespace RegistryService {
  class ApplicationRegistryClient;
  class FileSystemRegistryDataStore;
  class LocalRegistryDataStore;
  template<typename ServiceProtocolClientBuilderType> class RegistryClient;
  class RegistryDataStore;
  class RegistryDataStoreException;
  struct RegistryEntry;
  template<typename ContainerType, typename RegistryDataStoreType>
    class RegistryServlet;
  class RegistrySession;
  class VirtualRegistryClient;
  template<typename ClientType> class WrapperRegistryClient;

  // Standard name for the registry service.
  static const std::string SERVICE_NAME = "registry_service";
}
}

#endif
