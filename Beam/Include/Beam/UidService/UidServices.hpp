#ifndef BEAM_UID_SERVICES_HPP
#define BEAM_UID_SERVICES_HPP
#include <cstdint>
#include "Beam/Services/Service.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam::UidService {
  BEAM_DEFINE_SERVICES(UidServices,

  /**
   * Returns a unique block of uids.
   * @param block_size The size of the block to reserve.
   * @return The first uid in the reserved block.
   */
  (ReserveUidsService, "Beam.UidService.ReserveUidsService", std::uint64_t,
    std::uint64_t, block_size));
}

#endif
