#ifndef BEAM_BLANK_PARSER_HPP
#define BEAM_BLANK_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /**
   * Matches a single blank character (space or horizontal tab) from a stream.
   */
  class BlankParser {
    public:
      using Result = char;

      template<IsParserStream S>
      bool read(S& source, char& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of a BlankParser. */
  inline const auto blank_p = BlankParser();

  template<IsParserStream S>
  bool BlankParser::read(S& source, char& value) const {
    if(!source.read()) {
      return false;
    }
    value = source.peek();
    if(std::isblank(static_cast<unsigned char>(value))) {
      return true;
    }
    source.undo();
    return false;
  }

  template<IsParserStream S>
  bool BlankParser::read(S& source) const {
    if(!source.read()) {
      return false;
    }
    if(std::isblank(static_cast<unsigned char>(source.peek()))) {
      return true;
    }
    source.undo();
    return false;
  }
}

#endif
