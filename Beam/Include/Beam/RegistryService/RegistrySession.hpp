#ifndef BEAM_REGISTRY_SESSION_HPP
#define BEAM_REGISTRY_SESSION_HPP
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/RegistryService/RegistryService.hpp"

namespace Beam::RegistryService {

  /** Stores session info for a RegistryServlet Channel. */
  class RegistrySession : public ServiceLocator::AuthenticatedSession {};
}

#endif
