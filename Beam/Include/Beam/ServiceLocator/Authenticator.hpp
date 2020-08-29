#ifndef BEAM_AUTHENTICATOR_HPP
#define BEAM_AUTHENTICATOR_HPP
#include <functional>
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /**
   * Specifies the function type used to authenticate a session.
   * @param <C> The type of ServiceProtocolClient to authenticate.
   */
  template<typename C>
  struct Authenticator {
    using type = std::function<void(C&)>;
  };

  /**
   * Opens a ServiceProtocolClient and authenticates the session.
   * @param authenticator The Authenticator to use.
   * @param serviceProtocolClient The ServiceProtocolClient to authenticate.
   */
  template<typename Authenticator, typename ServiceProtocolClient>
  void OpenAndAuthenticate(const Authenticator& authenticator,
      ServiceProtocolClient& serviceProtocolClient) {
    authenticator(serviceProtocolClient);
  }
}

#endif
