#ifndef BEAM_SERVICELOCATOR_HPP
#define BEAM_SERVICELOCATOR_HPP

namespace Beam {
namespace ServiceLocator {
  class ApplicationServiceLocatorClient;
  class AuthenticatedSession;
  class AuthenticationException;
  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType> class AuthenticationServletAdapter;
  template<typename ServiceProtocolClientType> struct Authenticator;
  template<typename DataStoreType> class CachedServiceLocatorDataStore;
  struct DirectoryEntry;
  class LocalServiceLocatorDataStore;
  class MySqlServiceLocatorDataStore;
  class NotLoggedInException;
  class NullAuthenticator;
  class ServiceEntry;
  template<typename ServiceProtocolClientBuilderType>
    class ServiceLocatorClient;
  struct ServiceLocatorClientConfig;
  class ServiceLocatorDataStore;
  class ServiceLocatorDataStoreException;
  template<typename ContainerType, typename ServiceLocatorDataStoreType>
    class ServiceLocatorServlet;
  template<typename ServiceLocatorClientType> class SessionAuthenticator;
  class VirtualServiceLocatorClient;
  template<typename ClientType> class WrapperServiceLocatorClient;
}
}

#endif
