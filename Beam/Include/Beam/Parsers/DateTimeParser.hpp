#ifndef BEAM_DATETIMEPARSER_HPP
#define BEAM_DATETIMEPARSER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Parsers/CustomParser.hpp"
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"
#include "Beam/Parsers/Types.hpp"

namespace Beam {
namespace Parsers {

  /*! \class DateTimeParser
      \brief Matches a date/time in the form of yyyy-mm-dd hh:mm:ss
   */
  class DateTimeParser : public CustomParser<boost::posix_time::ptime> {
    public:

      //! Constructs a DateTimeParser.
      DateTimeParser();

    private:
      static boost::posix_time::ptime ToDateTime(const std::tuple<
        boost::gregorian::date, boost::posix_time::time_duration>& source);
  };

  inline DateTimeParser::DateTimeParser() {
    SetRule(Convert(DateParser() >> ' ' >> TimeDurationParser(), ToDateTime));
  }

  inline boost::posix_time::ptime DateTimeParser::ToDateTime(const std::tuple<
        boost::gregorian::date, boost::posix_time::time_duration>& source) {
    return boost::posix_time::ptime(std::get<0>(source), std::get<1>(source));
  }
}
}

#endif
