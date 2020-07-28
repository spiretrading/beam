#ifndef BEAM_SERVICE_LOCATOR_HPP
#define BEAM_SERVICE_LOCATOR_HPP

namespace Beam::ServiceLocator {
  class ApplicationServiceLocatorClient;
  class AuthenticatedSession;
  class AuthenticationException;
  template<typename C, typename S, typename L>
    class AuthenticationServletAdapter;
  template<typename C> struct Authenticator;
  template<typename D> class CachedServiceLocatorDataStore;
  struct DirectoryEntry;
  class LocalServiceLocatorDataStore;
  class NotLoggedInException;
  class NullAuthenticator;
  class ServiceEntry;
  template<typename B> class ServiceLocatorClient;
  struct ServiceLocatorClientConfig;
  class ServiceLocatorDataStore;
  class ServiceLocatorDataStoreException;
  template<typename C, typename D> class ServiceLocatorServlet;
  template<typename C> class SessionAuthenticator;
  template<typename C> class SqlServiceLocatorDataStore;
  class VirtualServiceLocatorClient;
  template<typename C> class WrapperServiceLocatorClient;
}

#endif
