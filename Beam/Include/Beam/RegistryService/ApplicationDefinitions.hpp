#ifndef BEAM_REGISTRY_APPLICATION_DEFINITIONS_HPP
#define BEAM_REGISTRY_APPLICATION_DEFINITIONS_HPP
#include "Beam/RegistryService/RegistryClient.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/Services/ApplicationDefinitions.hpp"

namespace Beam::RegistryService {

  /** Encapsulates a standard RegistryClient used in an application. */
  using ApplicationRegistryClient = Services::ApplicationClient<RegistryClient,
    Services::ServiceName<SERVICE_NAME>>;
}

#endif
