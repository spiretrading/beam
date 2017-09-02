#ifndef BEAM_DATEPARSER_HPP
#define BEAM_DATEPARSER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/CustomParser.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/Types.hpp"

namespace Beam {
namespace Parsers {

  /*! \class DateParser
      \brief Matches a date in the form yyyy-mm-dd.
   */
  class DateParser : public CustomParser<boost::gregorian::date> {
    public:

      //! Constructs a DateParser.
      DateParser();

    private:
      static boost::gregorian::date ToDate(
        const std::tuple<int, int, int>& source);
  };

  inline DateParser::DateParser() {
    SetRule(Convert(int_p >> '-' >> int_p >> '-' >> int_p, ToDate));
  }

  inline boost::gregorian::date DateParser::ToDate(
      const std::tuple<int, int, int>& source) {
    return boost::gregorian::date(
      static_cast<unsigned short>(std::get<0>(source)),
      static_cast<unsigned short>(std::get<1>(source)),
      static_cast<unsigned short>(std::get<2>(source)));
  }
}
}

#endif
