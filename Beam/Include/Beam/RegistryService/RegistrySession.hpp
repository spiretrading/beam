#ifndef BEAM_REGISTRYSESSION_HPP
#define BEAM_REGISTRYSESSION_HPP
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/RegistryService/RegistryService.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class RegistrySession
      \brief Stores session info for a RegistryServlet Channel.
   */
  class RegistrySession : public ServiceLocator::AuthenticatedSession {
    public:

      //! Constructs a RegistrySession.
      RegistrySession();
  };

  inline RegistrySession::RegistrySession() {}
}
}

#endif
