#ifndef BEAM_QUERY_STANDARD_VALUES_HPP
#define BEAM_QUERY_STANDARD_VALUES_HPP
#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/Queries/Value.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"

namespace Beam {
  using BoolValue = NativeValue<bool>;
  using CharValue = NativeValue<char>;
  using IntValue = NativeValue<int>;
  using DecimalValue = NativeValue<double>;
  using IdValue = NativeValue<std::uint64_t>;
  using StringValue = NativeValue<std::string>;
  using DateTimeValue = NativeValue<boost::posix_time::ptime>;
  using DurationValue = NativeValue<boost::posix_time::time_duration>;
}

#endif
