#ifndef BEAM_BLANK_PARSER_HPP
#define BEAM_BLANK_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /** Matches a blank character. */
  class BlankParser {
    public:
      using Result = char;

      template<typename Stream>
      bool Read(Stream& source, char& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool BlankParser::Read(Stream& source, char& value) const {
    if(!source.Read()) {
      return false;
    }
    value = source.GetChar();
    if(std::isblank(value)) {
      return true;
    }
    source.Undo();
    return false;
  }

  template<typename Stream>
  bool BlankParser::Read(Stream& source) const {
    if(!source.Read()) {
      return false;
    }
    if(std::isblank(source.GetChar())) {
      return true;
    }
    source.Undo();
    return false;
  }
}

#endif
