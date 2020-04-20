#ifndef BEAM_PARSERTYPES_HPP
#define BEAM_PARSERTYPES_HPP
#include <cstdint>
#include "Beam/Parsers/AlphaParser.hpp"
#include "Beam/Parsers/AnyParser.hpp"
#include "Beam/Parsers/BoolParser.hpp"
#include "Beam/Parsers/DecimalParser.hpp"
#include "Beam/Parsers/DigitParser.hpp"
#include "Beam/Parsers/DiscardParser.hpp"
#include "Beam/Parsers/EpsilonParser.hpp"
#include "Beam/Parsers/IntegralParser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SpaceParser.hpp"
#include "Beam/Parsers/StringParser.hpp"
#include "Beam/Parsers/TokenParser.hpp"
#include "Beam/Parsers/Operators.hpp"

namespace Beam::Parsers {
  static const AlphaParser alpha_p = AlphaParser{};
  static const AnyParser any_p = AnyParser{};
  static const BoolParser bool_p = BoolParser{};
  static const DecimalParser<double> double_p = DecimalParser<double>{};
  static const DigitParser digit_p = DigitParser{};
  static const EpsilonParser eps_p = EpsilonParser{};
  static const IntegralParser<int> int_p = IntegralParser<int>{};
  static const IntegralParser<std::int64_t> int64_p =
    IntegralParser<std::int64_t>{};
  static const auto space_p = Discard(SpaceParser());
  static const StringParser string_p = StringParser{};
  static const auto tokenize = ChainTokenParser<EpsilonParser>(EpsilonParser());
}

#endif
