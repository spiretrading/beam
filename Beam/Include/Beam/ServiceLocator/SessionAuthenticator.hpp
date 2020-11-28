#ifndef BEAM_SESSION_AUTHENTICATOR_HPP
#define BEAM_SESSION_AUTHENTICATOR_HPP
#include <type_traits>
#include <cryptopp/osrng.h>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/Authenticator.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"

namespace Beam::ServiceLocator {

  /**
   * Used to authenticate a ServiceProtocolClient's session.
   * @param <C> The type of ServiceLocatorClient used to authenticate the
   *            session.
   */
  template<typename C>
  class SessionAuthenticator {
    public:

      /** The type of ServiceLocatorClient used to authenticate the session. */
      using ServiceLocatorClient = GetTryDereferenceType<C>;

      /**
       * Constructs a SessionAuthenticator.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the session.
       */
      template<typename CF>
      explicit SessionAuthenticator(CF&& serviceLocatorClient);

      template<typename ServiceProtocolClient>
      void operator ()(ServiceProtocolClient& client) const;

    private:
      GetOptionalLocalPtr<C> m_serviceLocatorClient;
  };

  template<typename C>
  SessionAuthenticator(C&&) -> SessionAuthenticator<std::decay_t<C>>;

  template<typename C>
  template<typename CF>
  SessionAuthenticator<C>::SessionAuthenticator(CF&& serviceLocatorClient)
    : m_serviceLocatorClient(std::forward<CF>(serviceLocatorClient)) {}

  template<typename C>
  template<typename ServiceProtocolClient>
  void SessionAuthenticator<C>::operator ()(
      ServiceProtocolClient& client) const {
    auto randomGenerator = CryptoPP::AutoSeededRandomPool();
    auto key = int();
    randomGenerator.GenerateBlock(reinterpret_cast<unsigned char*>(&key),
      sizeof(key));
    RegisterServiceLocatorServices(Store(client.GetSlots()));
    client.template SendRequest<SendSessionIdService>(key,
      m_serviceLocatorClient->GetEncryptedSessionId(key));
  }
}

#endif
