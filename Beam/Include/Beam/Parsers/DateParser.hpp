#ifndef BEAM_DATEPARSER_HPP
#define BEAM_DATEPARSER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /** Parses a date in the form yyyy-mm-dd. */
  inline auto DateParser() {
    return Convert(IntegralParser<int>() >> '-' >> IntegralParser<int>() >>
      '-' >> IntegralParser<int>(),
      [] (const std::tuple<int, int, int>& source) {
        return boost::gregorian::date(
          static_cast<unsigned short>(std::get<0>(source)),
          static_cast<unsigned short>(std::get<1>(source)),
          static_cast<unsigned short>(std::get<2>(source)));
      });
  }
}

#endif
