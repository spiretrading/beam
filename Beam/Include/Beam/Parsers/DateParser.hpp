#ifndef BEAM_DATE_PARSER_HPP
#define BEAM_DATE_PARSER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"

namespace Beam {

  /** Parses a date in the form yyyy-mm-dd. */
  inline auto date_parser() {
    return convert(int_p >> '-' >> int_p >> '-' >> int_p,
      [] (const std::tuple<int, int, int>& source) {
        return boost::gregorian::date(
          static_cast<unsigned short>(std::get<0>(source)),
          static_cast<unsigned short>(std::get<1>(source)),
          static_cast<unsigned short>(std::get<2>(source)));
      });
  }

  /** A parser that parses a date in the form yyyy-mm-dd. */
  inline const auto date_p = date_parser();
}

#endif
