#ifndef BEAM_AUTHENTICATOR_HPP
#define BEAM_AUTHENTICATOR_HPP
#include <functional>
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class Authenticator
      \brief Specifies the function type used to authenticate a session.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClient to
              authenticate.
   */
  template<typename ServiceProtocolClientType>
  struct Authenticator {
    typedef std::function<void (ServiceProtocolClientType& client)> type;
  };

  //! Opens a ServiceProtocolClient and authenticates the session.
  /*!
    \param authenticator The Authenticator to use.
    \param serviceProtocolClient The ServiceProtocolClient to authenticate.
  */
  template<typename Authenticator, typename ServiceProtocolClient>
  void OpenAndAuthenticate(const Authenticator& authenticator,
      ServiceProtocolClient& serviceProtocolClient) {
    serviceProtocolClient.Open();
    authenticator(serviceProtocolClient);
  }
}
}

#endif
