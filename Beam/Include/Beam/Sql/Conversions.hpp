#ifndef BEAM_SQL_CONVERSIONS_HPP
#define BEAM_SQL_CONVERSIONS_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Viper/Conversions.hpp>
#include <Viper/DataTypes/NativeToDataType.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Viper {
  template<>
  inline const auto native_to_data_type_v<boost::posix_time::time_duration> =
    big_int;

  template<>
  inline const auto native_to_data_type_v<Beam::Sequence> =
    native_to_data_type_v<Beam::Sequence::Ordinal>;

  template<typename T, std::size_t N>
  inline const auto native_to_data_type_v<Beam::Enum<T, N>> = integer;

  template<>
  inline const auto native_to_data_type_v<Beam::SharedBuffer> = blob();

  template<>
  struct ToSql<boost::posix_time::time_duration> {
    void operator ()(
        boost::posix_time::time_duration value, std::string& column) const {
      to_sql(static_cast<std::int64_t>(value.total_microseconds()), column);
    }
  };

  template<>
  struct FromSql<boost::posix_time::time_duration> {
    boost::posix_time::time_duration operator ()(
        const RawColumn& column) const {
      return boost::posix_time::microseconds(from_sql<std::int64_t>(column));
    }
  };

  template<>
  struct ToSql<Beam::Sequence> {
    void operator ()(Beam::Sequence value, std::string& column) const {
      to_sql(value.get_ordinal(), column);
    }
  };

  template<>
  struct FromSql<Beam::Sequence> {
    Beam::Sequence operator ()(const RawColumn& column) const {
      return Beam::Sequence(from_sql<Beam::Sequence::Ordinal>(column));
    }
  };

  template<typename T, std::size_t N>
  struct ToSql<Beam::Enum<T, N>> {
    void operator ()(Beam::Enum<T, N> value, std::string& column) const {
      to_sql(static_cast<typename T::Type>(value), column);
    }
  };

  template<typename T, std::size_t N>
  struct FromSql<Beam::Enum<T, N>> {
    auto operator ()(const RawColumn& column) const {
      return Beam::Enum<T, N>(from_sql<typename T::Type>(column));
    }
  };

  template<std::size_t N>
  struct ToSql<Beam::FixedString<N>> {
    void operator ()(Beam::FixedString<N> value, std::string& column) const {
      to_sql(std::string(value.get_data()), column);
    }
  };

  template<std::size_t N>
  struct FromSql<Beam::FixedString<N>> {
    auto operator ()(const RawColumn& column) const {
      return Beam::FixedString<N>(
        std::string_view(column.m_data, std::strlen(column.m_data)));
    }
  };

  template<>
  struct ToSql<Beam::SharedBuffer> {
    void operator ()(
        const Beam::SharedBuffer& value, std::string& column) const {
      auto blob = std::vector<std::byte>(value.get_size());
      std::memcpy(blob.data(), value.get_data(), value.get_size());
      to_sql(blob, column);
    }
  };

  template<>
  struct FromSql<Beam::SharedBuffer> {
    auto operator ()(const RawColumn& column) const {
      return Beam::SharedBuffer(column.m_data, column.m_size);
    }
  };
}

#endif
