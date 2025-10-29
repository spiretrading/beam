#ifndef BEAM_UID_APPLICATION_DEFINITIONS_HPP
#define BEAM_UID_APPLICATION_DEFINITIONS_HPP
#include "Beam/Services/ApplicationDefinitions.hpp"
#include "Beam/UidService/ServiceUidClient.hpp"

namespace Beam {

  /** Encapsulates a standard UidClient used in an application. */
  using ApplicationUidClient =
    ApplicationClient<ServiceUidClient, ServiceName<UID_SERVICE_NAME>>;
}

#endif
