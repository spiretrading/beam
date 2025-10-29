#ifndef BEAM_ALPHA_PARSER_HPP
#define BEAM_ALPHA_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /** Matches an alphabet character. */
  class AlphaParser {
    public:
      using Result = char;

      template<IsParserStream S>
      bool read(S& source, char& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of an AlphaParser. */
  inline const auto alpha_p = AlphaParser();

  template<IsParserStream S>
  bool AlphaParser::read(S& source, char& value) const {
    if(!source.read()) {
      return false;
    }
    value = source.peek();
    if(std::isalpha(value)) {
      return true;
    }
    source.undo();
    return false;
  }

  template<IsParserStream S>
  bool AlphaParser::read(S& source) const {
    if(!source.read()) {
      return false;
    }
    if(std::isalpha(source.peek())) {
      return true;
    }
    source.undo();
    return false;
  }
}

#endif
