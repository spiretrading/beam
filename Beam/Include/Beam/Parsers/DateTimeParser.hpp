#ifndef BEAM_DATE_TIME_PARSER_HPP
#define BEAM_DATE_TIME_PARSER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"

namespace Beam {

  /** Matches a date/time in the form of yyyy-mm-dd hh:mm:ss */
  inline auto date_time_parser() {
    return convert(date_p >> ' ' >> time_duration_p,
      [] (const std::tuple<boost::gregorian::date,
          boost::posix_time::time_duration>& source) {
        return boost::posix_time::ptime(std::get<0>(source),
          std::get<1>(source));
      });
  }

  /** A parser that matches a date/time in the form of yyyy-mm-dd hh:mm:ss */
  inline const auto date_time_p = date_time_parser();
}

#endif
