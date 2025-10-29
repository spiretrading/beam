#ifndef BEAM_DIGIT_PARSER_HPP
#define BEAM_DIGIT_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /** Matches a digit character. */
  class DigitParser {
    public:
      using Result = char;

      template<IsParserStream S>
      bool read(S& source, char& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of a DigitParser. */
  inline const auto digit_p = DigitParser();

  template<IsParserStream S>
  bool DigitParser::read(S& source, char& value) const {
    if(!source.read()) {
      return false;
    }
    value = source.peek();
    if(std::isdigit(value)) {
      return true;
    }
    source.undo();
    return false;
  }

  template<IsParserStream S>
  bool DigitParser::read(S& source) const {
    if(!source.read()) {
      return false;
    }
    if(std::isdigit(source.peek())) {
      return true;
    }
    source.undo();
    return false;
  }
}

#endif
