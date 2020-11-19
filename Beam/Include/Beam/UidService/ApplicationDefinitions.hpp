#ifndef BEAM_UID_APPLICATION_DEFINITIONS_HPP
#define BEAM_UID_APPLICATION_DEFINITIONS_HPP
#include "Beam/Services/ApplicationDefinitions.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam::UidService {

  /** Encapsulates a standard UidClient used in an application. */
  using ApplicationUidClient = Services::ApplicationClient<UidClient,
    Services::ServiceName<SERVICE_NAME>>;
}

#endif
