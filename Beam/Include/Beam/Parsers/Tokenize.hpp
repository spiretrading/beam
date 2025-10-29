#ifndef BEAM_TOKENIZE_HPP
#define BEAM_TOKENIZE_HPP
#include <utility>
#include "Beam/Parsers/ConcatenateParser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"

namespace Beam {

  /**
   * Returns a parser that matches the specified parsers with optional spaces
   * between them.
   * @param parsers The sequence of Parsers to match.
   */
  template<typename... Ps>
  auto tokenize(Ps&&... parsers) requires (
      (IsParser<std::remove_cvref_t<Ps>> || IsParser<to_parser_t<Ps>>) && ...) {
    if constexpr(sizeof...(Ps) == 0) {
      return SkipSpaceParser();
    } else {
      return (SkipSpaceParser() >> ... >>
        (std::forward<Ps>(parsers) >> SkipSpaceParser()));
    }
  }
}

#endif
