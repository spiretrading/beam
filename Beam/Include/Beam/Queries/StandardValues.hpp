#ifndef BEAM_QUERYSTANDARDVALUES_HPP
#define BEAM_QUERYSTANDARDVALUES_HPP
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

namespace Beam {
namespace Queries {
  using BoolValue = NativeValue<BoolType>;
  using CharValue = NativeValue<CharType>;
  using IntValue = NativeValue<IntType>;
  using DecimalValue = NativeValue<DecimalType>;
  using IdValue = NativeValue<IdType>;
  using StringValue = NativeValue<StringType>;
  using DateTimeValue = NativeValue<DateTimeType>;
  using DurationValue = NativeValue<DurationType>;
}
}

#endif
