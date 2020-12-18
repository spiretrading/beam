#ifndef BEAM_QUERY_STANDARD_DATA_TYPES_HPP
#define BEAM_QUERY_STANDARD_DATA_TYPES_HPP
#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam::Queries {
  using BoolType = NativeDataType<bool>;
  using CharType = NativeDataType<char>;
  using IntType = NativeDataType<int>;
  using DecimalType = NativeDataType<double>;
  using IdType = NativeDataType<std::uint64_t>;
  using StringType = NativeDataType<std::string>;
  using DateTimeType = NativeDataType<boost::posix_time::ptime>;
  using DurationType = NativeDataType<boost::posix_time::time_duration>;

  /** A variant able to represent any query type. */
  using  QueryVariant = boost::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;

  /** Wraps a QueryVariant into a SequencedValue. */
  using SequencedQueryVariant = SequencedValue<QueryVariant>;

  /** Stores typedefs of various types that can be used in an Expression. */
  struct QueryTypes {

    /** Lists all native types. */
    using NativeTypes = boost::mpl::list<bool, char, int, double, std::uint64_t,
      std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;

    /** Lists all value types. */
    using ValueTypes = boost::mpl::list<bool, char, int, double, std::uint64_t,
      std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;

    /** Lists types that can be compared. */
    using ComparableTypes = boost::mpl::list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;
  };
}

#endif
