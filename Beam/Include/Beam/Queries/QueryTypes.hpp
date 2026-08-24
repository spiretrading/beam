#ifndef BEAM_QUERY_TYPES_HPP
#define BEAM_QUERY_TYPES_HPP
#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/mp11.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {

  /** The maximum number of supported parameters. */
  constexpr auto MAX_EVALUATOR_PARAMETERS = 2;

  /** A variant able to represent any query type. */
  using QueryVariant = boost::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;

  /** Wraps a QueryVariant into a SequencedValue. */
  using SequencedQueryVariant = SequencedValue<QueryVariant>;

  /** Stores typedefs of various types that can be used in an Expression. */
  struct QueryTypes {

    /** Lists all native types. */
    using NativeTypes = boost::mp11::mp_list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;

    /** Lists the types that can be used as a constant. */
    using ValueTypes = boost::mp11::mp_list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;

    /** Lists types that can be compared. */
    using ComparableTypes = boost::mp11::mp_list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;
  };
}

#endif
