#ifndef BEAM_PARSERTYPES_HPP
#define BEAM_PARSERTYPES_HPP
#include <cstdint>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/BoolParser.hpp"
#include "Beam/Parsers/DateParser.hpp"
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/DiscardParser.hpp"
#include "Beam/Parsers/EpsilonParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/RationalParser.hpp"
#include "Beam/Parsers/SpaceParser.hpp"
#include "Beam/Parsers/StringParser.hpp"
#include "Beam/Parsers/TimeDurationParser.hpp"
#include "Beam/Parsers/TokenParser.hpp"

namespace Beam::Parsers {

  /** Specifies the default parser to use for a given type. */
  template<typename T, typename = void>
  inline const auto default_parser = std::enable_if_t<!std::is_same_v<T, T>>();

  template<typename T>
  inline const auto default_parser<std::vector<T>> = [] {
    return '[' >> List(default_parser<T>, ',') >> ']';
  }();

  template<>
  inline const auto default_parser<char> = AnyParser();

  template<>
  inline const auto default_parser<bool> = BoolParser();

  template<>
  inline const auto default_parser<int> = IntegralParser<int>();

  template<>
  inline const auto default_parser<std::int64_t> = IntegralParser<std::int64_t>();

  template<>
  inline const auto default_parser<double> = DecimalParser<double>();

  template<>
  inline const auto default_parser<boost::gregorian::date> = DateParser();

  template<>
  inline const auto default_parser<boost::posix_time::ptime> = DateTimeParser();

  template<>
  inline const auto default_parser<boost::posix_time::time_duration> =
    TimeDurationParser();

  template<typename T>
  inline const auto default_parser<boost::rational<T>> = RationalParser<T>();

  template<>
  inline const auto default_parser<std::string> = StringParser();

  static const auto alpha_p = AlphaParser();
  static const auto any_p = default_parser<char>;
  static const auto bool_p = default_parser<bool>;
  static const auto digit_p = DigitParser{};
  static const auto double_p = default_parser<double>;
  static const auto eps_p = EpsilonParser();
  static const auto int_p = default_parser<int>;
  static const auto int64_p = default_parser<std::int64_t>;
  static const auto rational_p = default_parser<boost::rational<int>>;
  static const auto space_p = Discard(SpaceParser());
  static const auto string_p = default_parser<std::string>;
  static const auto tokenize = ChainTokenParser<EpsilonParser>(EpsilonParser());
}

#endif
