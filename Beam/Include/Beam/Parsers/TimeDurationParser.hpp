#ifndef BEAM_TIME_DURATION_PARSER_HPP
#define BEAM_TIME_DURATION_PARSER_HPP
#include <cstdint>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /** Matches a time duration. */
  inline auto TimeDurationParser() {
    return Convert(IntegralParser<int>() >> ':' >> IntegralParser<int>() >>
      ':' >> DecimalParser<double>(),
      [] (const std::tuple<int, int, double>& source) {
        return boost::posix_time::time_duration(std::get<0>(source),
          std::get<1>(source), 0, static_cast<std::uint64_t>(
          boost::posix_time::time_duration::ticks_per_second() *
          std::get<2>(source)));
      });
  }
}

#endif
