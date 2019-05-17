#ifndef BEAM_SQL_CONVERSIONS_HPP
#define BEAM_SQL_CONVERSIONS_HPP
#include <Viper/Conversions.hpp>
#include <Viper/DataTypes/NativeToDataType.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Sql/Sql.hpp"

namespace Viper {
  template<typename T, std::size_t N>
  inline const auto native_to_data_type_v<Beam::Enum<T, N>> = integer;

  template<>
  inline const auto native_to_data_type_v<Beam::IO::SharedBuffer> = blob();

  template<typename T, std::size_t N>
  struct ToSql<Beam::Enum<T, N>> {
    void operator ()(Beam::Enum<T, N> value, std::string& column) const {
      to_sql(static_cast<std::int32_t>(value), column);
    }
  };

  template<typename T, std::size_t N>
  struct FromSql<Beam::Enum<T, N>> {
    auto operator ()(const RawColumn& column) const {
      return Beam::Enum<T, N>(from_sql<std::int32_t>(column));
    }
  };

  template<>
  struct ToSql<Beam::IO::SharedBuffer> {
    void operator ()(const Beam::IO::SharedBuffer& value,
        std::string& column) const {
      auto blob = std::vector<std::byte>(value.GetSize());
      std::memcpy(blob.data(), value.GetData(), value.GetSize());
      to_sql(blob, column);
    }
  };

  template<>
  struct FromSql<Beam::IO::SharedBuffer> {
    auto operator ()(const RawColumn& column) const {
      return Beam::IO::SharedBuffer(column.m_data, column.m_size);
    }
  };
}

#endif
