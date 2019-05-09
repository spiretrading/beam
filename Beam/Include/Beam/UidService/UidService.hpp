#ifndef BEAM_UID_SERVICE_HPP
#define BEAM_UID_SERVICE_HPP

namespace Beam::UidService {
  class ApplicationUidClient;
  class LocalUidDataStore;
  template<typename C> class SqlUidDataStore;
  template<typename ServiceProtocolClientBuilderType> class UidClient;
  class UidDataStore;
  class UidServiceException;
  template<typename ContainerType, typename UidDataStoreType> class UidServlet;
  class VirtualUidClient;
  template<typename ClientType> class WrapperUidClient;

  // Standard name for the uid service.
  constexpr auto SERVICE_NAME = "uid_service";
}

#endif
