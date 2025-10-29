#ifndef BEAM_SESSION_AUTHENTICATOR_HPP
#define BEAM_SESSION_AUTHENTICATOR_HPP
#include <type_traits>
#include <cryptopp/osrng.h>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/ServiceLocator/Authenticator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"

namespace Beam {

  /**
   * Used to authenticate a ServiceProtocolClient's session.
   * @tparam C The type of ServiceLocatorClient used to authenticate the
   *            session.
   */
  template<IsServiceLocatorClient C>
  class SessionAuthenticator {
    public:

      /** The type of ServiceLocatorClient used to authenticate the session. */
      using ServiceLocatorClient = C;

      /**
       * Constructs a SessionAuthenticator.
       * @param client The ServiceLocatorClient used to authenticate the
       *        session.
       */
      explicit SessionAuthenticator(Ref<ServiceLocatorClient> client);

      template<typename M, typename T, typename P, typename S, bool V>
      void operator ()(ServiceProtocolClient<M, T, P, S, V>& client) const;

    private:
      ServiceLocatorClient* m_client;
  };

  template<IsServiceLocatorClient C>
  SessionAuthenticator<C>::SessionAuthenticator(
    Ref<ServiceLocatorClient> client)
    : m_client(client.get()) {}

  template<IsServiceLocatorClient C>
  template<typename M, typename T, typename P, typename S, bool V>
  void SessionAuthenticator<C>::operator ()(
      ServiceProtocolClient<M, T, P, S, V>& client) const {
    auto random_generator = CryptoPP::AutoSeededRandomPool();
    auto key = int();
    random_generator.GenerateBlock(
      reinterpret_cast<unsigned char*>(&key), sizeof(key));
    ServiceLocatorServices::register_service_locator_services(
      out(client.get_slots()));
    client.template send_request<ServiceLocatorServices::SendSessionIdService>(
      key, m_client->get_encrypted_session_id(key));
  }
}

#endif
