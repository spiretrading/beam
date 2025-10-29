#ifndef BEAM_ANY_PARSER_HPP
#define BEAM_ANY_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /** Matches any character. */
  class AnyParser {
    public:
      using Result = char;

      template<IsParserStream S>
      bool read(S& source, char& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of an AnyParser. */
  inline const auto any_p = AnyParser();

  template<IsParserStream S>
  bool AnyParser::read(S& source, char& value) const {
    if(!source.read()) {
      return false;
    }
    value = source.peek();
    return true;
  }

  template<IsParserStream S>
  bool AnyParser::read(S& source) const {
    return source.read();
  }
}

#endif
