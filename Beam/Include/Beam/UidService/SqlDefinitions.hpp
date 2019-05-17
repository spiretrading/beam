#ifndef BEAM_UID_SQL_DEFINITIONS_HPP
#define BEAM_UID_SQL_DEFINITIONS_HPP
#include <cstdint>
#include <Viper/Row.hpp>
#include "Beam/UidService/UidService.hpp"

namespace Beam::UidService {

  //! Returns a row representing the next UID.
  inline const auto& GetNextUidRow() {
    static auto ROW = Viper::Row<std::uint64_t>("uid");
    return ROW;
  }
}

#endif
