#ifndef BEAM_PARSEROPERATORS_HPP
#define BEAM_PARSEROPERATORS_HPP
#include <type_traits>
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/DifferenceParser.hpp"
#include "Beam/Parsers/OrParser.hpp"
#include "Beam/Parsers/PlusParser.hpp"
#include "Beam/Parsers/StarParser.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {
namespace Details {
  template<typename L, typename R, typename = void>
  struct has_right_stream_operator : std::false_type {};

  template<typename L, typename R>
  struct has_right_stream_operator<L, R, std::void_t<
      decltype(&L::template operator >><to_parser_t<R>>)>> :
    std::true_type {};
}

  template<typename L, typename R, typename = std::enable_if_t<
    is_parser_v<to_parser_t<L>> && is_parser_v<to_parser_t<R>> &&
    !Details::has_right_stream_operator<to_parser_t<L>, R>::value>>
  auto operator >>(L left, R right) {
    return ConcatenateParser(std::move(left), std::move(right));
  }

  template<typename L, typename R, typename = std::enable_if_t<
    is_parser_v<to_parser_t<L>> && is_parser_v<to_parser_t<R>>>>
  auto operator -(L left, R right) {
    return DifferenceParser(std::move(left), std::move(right));
  }

  template<typename L, typename R, typename = std::enable_if_t<
    is_parser_v<to_parser_t<L>> && is_parser_v<to_parser_t<R>>>>
  auto operator |(L left, R right) {
    return OrParser(std::move(left), std::move(right));
  }

  template<typename P, typename = std::enable_if_t<is_parser_v<P>>>
  auto operator *(P parser) {
    return StarParser(std::move(parser));
  }

  template<typename P, typename = std::enable_if_t<is_parser_v<P>>>
  auto operator +(P parser) {
    return PlusParser(std::move(parser));
  }
}

#endif
