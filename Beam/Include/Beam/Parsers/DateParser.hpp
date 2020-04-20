#ifndef BEAM_DATEPARSER_HPP
#define BEAM_DATEPARSER_HPP
#include <tuple>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/Types.hpp"

namespace Beam::Parsers {

  /** Parses a date in the form yyyy-mm-dd. */
  inline auto DateParser() {
    return Convert(int_p >> '-' >> int_p >> '-' >> int_p,
      [] (const std::tuple<int, int, int>& source) {
        return boost::gregorian::date(
          static_cast<unsigned short>(std::get<0>(source)),
          static_cast<unsigned short>(std::get<1>(source)),
          static_cast<unsigned short>(std::get<2>(source)));
      });
  }
}

#endif
