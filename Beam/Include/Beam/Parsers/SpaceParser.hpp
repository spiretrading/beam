#ifndef BEAM_SPACE_PARSER_HPP
#define BEAM_SPACE_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /** Matches a blank character. */
  class SpaceParser {
    public:
      using Result = char;

      template<IsParserStream S>
      bool read(S& source, char& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  template<IsParserStream S>
  bool SpaceParser::read(S& source, char& value) const {
    if(!source.read()) {
      return false;
    }
    value = source.peek();
    if(std::isspace(value)) {
      return true;
    }
    source.undo();
    return false;
  }

  template<IsParserStream S>
  bool SpaceParser::read(S& source) const {
    if(!source.read()) {
      return false;
    }
    if(std::isspace(source.peek())) {
      return true;
    }
    source.undo();
    return false;
  }
}

#endif
