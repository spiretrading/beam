#ifndef BEAM_UIDSERVICES_HPP
#define BEAM_UIDSERVICES_HPP
#include <cstdint>
#include "Beam/Services/Service.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam {
namespace UidService {
  BEAM_DEFINE_SERVICES(UidServices,

  /*! \interface Beam::UidService::ReserveUidsService
      \brief Returns a unique block of uids.
      \param block_size <code>std::uint64_t</code> The size of the block to
             reserve.
      \return <code>std::uint64_t</code> The first uid in the reserved block.
  */
  //! \cond
  (ReserveUidsService, "Beam.UidService.ReserveUidsService", std::uint64_t,
    std::uint64_t, block_size));
  //! \endcond
}
}

#endif
