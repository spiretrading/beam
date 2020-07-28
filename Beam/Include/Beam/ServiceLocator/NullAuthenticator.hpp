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
      template<typename ServiceProtocolClientType>
      void operator ()(ServiceProtocolClientType& client) const;
  };

  template<typename ServiceProtocolClientType>
  void NullAuthenticator::operator ()(
    ServiceProtocolClientType& client) const {}
}

#endif
