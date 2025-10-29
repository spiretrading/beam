#ifndef BEAM_UID_SERVICES_HPP
#define BEAM_UID_SERVICES_HPP
#include <cstdint>
#include <string>
#include "Beam/Services/Service.hpp"

namespace Beam {

  /** Standard name for the uid service. */
  inline const auto UID_SERVICE_NAME = std::string("uid_service");

  BEAM_DEFINE_SERVICES(uid_services,

    /**
     * Returns a unique block of uids.
     * @param block_size The size of the block to reserve.
     * @return The first uid in the reserved block.
     */
    (ReserveUidsService, "Beam.UidService.ReserveUidsService", std::uint64_t,
      (std::uint64_t, block_size)));
}

#endif
