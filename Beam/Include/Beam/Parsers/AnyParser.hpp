#ifndef BEAM_ANY_PARSER_HPP
#define BEAM_ANY_PARSER_HPP
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /** Matches any character. */
  class AnyParser {
    public:
      using Result = char;

      template<typename Stream>
      bool Read(Stream& source, char& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool AnyParser::Read(Stream& source, char& value) const {
    if(!source.Read()) {
      return false;
    }
    value = source.GetChar();
    return true;
  }

  template<typename Stream>
  bool AnyParser::Read(Stream& source) const {
    return source.Read();
  }
}

#endif
