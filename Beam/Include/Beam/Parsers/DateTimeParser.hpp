#ifndef BEAM_DATETIMEPARSER_HPP
#define BEAM_DATETIMEPARSER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"

namespace Beam::Parsers {

  /** Matches a date/time in the form of yyyy-mm-dd hh:mm:ss */
  inline auto DateTimeParser() {
    return Convert(DateParser() >> ' ' >> TimeDurationParser(),
      [] (const std::tuple<boost::gregorian::date,
          boost::posix_time::time_duration>& source) {
        return boost::posix_time::ptime(std::get<0>(source),
          std::get<1>(source));
      });
  }
}

#endif
