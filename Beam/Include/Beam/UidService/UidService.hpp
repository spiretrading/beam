#ifndef BEAM_UIDSERVICE_HPP
#define BEAM_UIDSERVICE_HPP
#include <string>

namespace Beam {
namespace UidService {
  class ApplicationUidClient;
  class LocalUidDataStore;
  class MySqlUidDataStore;
  template<typename ServiceProtocolClientBuilderType> class UidClient;
  class UidDataStore;
  class UidServiceException;
  template<typename ContainerType, typename UidDataStoreType> class UidServlet;
  class VirtualUidClient;
  template<typename ClientType> class WrapperUidClient;

  // Standard name for the uid service.
  static const std::string SERVICE_NAME = "uid_service";
}
}

#endif
