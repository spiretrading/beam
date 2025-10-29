#ifndef BEAM_UID_SQL_DEFINITIONS_HPP
#define BEAM_UID_SQL_DEFINITIONS_HPP
#include <cstdint>
#include <Viper/Row.hpp>

namespace Beam {

  /** Returns a row representing the next UID. */
  inline const auto& get_next_uid_row() {
    static auto ROW = Viper::Row<std::uint64_t>("uid");
    return ROW;
  }
}

#endif
