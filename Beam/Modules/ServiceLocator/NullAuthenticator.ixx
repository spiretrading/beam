module;
#include "Prelude.hpp"

export module Beam:NullAuthenticator;

export namespace Beam {

  /**
   * Does not perform any authentication of a ServiceProtocolClient's
   * session.
   */
  class NullAuthenticator {
    public:
      template<typename M, typename T, typename P, typename S, bool V>
      void operator ()(ServiceProtocolClient<M, T, P, S, V>& client) const;
  };

  template<typename M, typename T, typename P, typename S, bool V>
  void NullAuthenticator::operator ()(
    ServiceProtocolClient<M, T, P, S, V>& client) const {}
}

