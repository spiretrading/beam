#ifndef BEAM_DEFAULT_PARSER_HPP
#define BEAM_DEFAULT_PARSER_HPP
#include <cstdint>
#include <type_traits>
#include <vector>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/BoolParser.hpp"
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/DiscardParser.hpp"
#include "Beam/Parsers/EpsilonParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/RationalParser.hpp"
#include "Beam/Parsers/SpaceParser.hpp"
#include "Beam/Parsers/StringParser.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"

namespace Beam {

  /** Specifies the default parser to use for a given type. */
  template<typename T>
  inline const auto default_parser = std::enable_if_t<!std::is_same_v<T, T>>();

  template<typename T>
  inline const auto default_parser<std::vector<T>> =
    '[' >> list(default_parser<T>, ',') >> ']';

  template<>
  inline const auto default_parser<char> = any_p;

  template<>
  inline const auto default_parser<bool> = bool_p;

  template<>
  inline const auto default_parser<int> = int_p;

  template<>
  inline const auto default_parser<std::int64_t> = int64_p;

  template<>
  inline const auto default_parser<double> = double_p;

  template<>
  inline const auto default_parser<boost::gregorian::date> = date_p;

  template<>
  inline const auto default_parser<boost::posix_time::ptime> = date_time_p;

  template<>
  inline const auto default_parser<boost::posix_time::time_duration> =
    time_duration_p;

  template<typename T>
  inline const auto default_parser<boost::rational<T>> = rational_p;

  template<>
  inline const auto default_parser<std::string> = string_p;
}

#endif
