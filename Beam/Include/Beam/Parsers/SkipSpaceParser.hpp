#ifndef BEAM_SKIP_SPACE_PARSER_HPP
#define BEAM_SKIP_SPACE_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"

namespace Beam {

  /** Parser used to skip spaces. */
  class SkipSpaceParser {
    public:
      using Result = void;

      template<IsParserStream S>
      bool read(S& source) const;
  };

  template<IsParserStream S>
  bool SkipSpaceParser::read(S& source) const {
    while(true) {
      if(!source.read()) {
        return true;
      }
      if(!std::isspace(source.peek())) {
        source.undo();
        return true;
      }
    }
    return true;
  }
}

#endif
