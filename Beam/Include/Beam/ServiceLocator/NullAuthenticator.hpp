#ifndef BEAM_NULL_AUTHENTICATOR_HPP
#define BEAM_NULL_AUTHENTICATOR_HPP
#include "Beam/ServiceLocator/Authenticator.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /**
   * Does not perform any authentication of a ServiceProtocolClient's
   * session.
   */
  class NullAuthenticator {
    public:
      template<typename ServiceProtocolClient>
      void operator ()(ServiceProtocolClient& client) const;
  };

  template<typename ServiceProtocolClient>
  void NullAuthenticator::operator ()(ServiceProtocolClient& client) const {}
}

#endif
