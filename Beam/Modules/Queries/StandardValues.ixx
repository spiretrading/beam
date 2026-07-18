module;
#include "Prelude.hpp"

export module Beam:StandardValues;

export namespace Beam {
  using BoolValue = NativeValue<bool>;
  using CharValue = NativeValue<char>;
  using IntValue = NativeValue<int>;
  using DecimalValue = NativeValue<double>;
  using IdValue = NativeValue<std::uint64_t>;
  using StringValue = NativeValue<std::string>;
  using DateTimeValue = NativeValue<boost::posix_time::ptime>;
  using DurationValue = NativeValue<boost::posix_time::time_duration>;
}

