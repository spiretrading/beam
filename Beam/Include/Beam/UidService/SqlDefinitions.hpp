#ifndef BEAM_UID_SQL_DEFINITIONS_HPP
#define BEAM_UID_SQL_DEFINITIONS_HPP
#include <Viper/Row.hpp>

namespace Beam::UidService {

  //! Returns a row representing the next UID.
  inline const auto& GetNextUidRow() {
    static auto ROW = Viper::Row<std::uint64_t>().
      add_column("uid", Viper::big_uint);
    return ROW;
  }
}

#endif
