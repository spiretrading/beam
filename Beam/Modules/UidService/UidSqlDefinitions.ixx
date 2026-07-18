module;
#include "Prelude.hpp"

export module Beam:UidSqlDefinitions;

export namespace Beam {

  /** Returns a row representing the next UID. */
  inline const auto& get_next_uid_row() {
    static auto ROW = Viper::Row<std::uint64_t>("uid");
    return ROW;
  }
}
