#ifndef BEAM_ANYPARSER_HPP
#define BEAM_ANYPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class AnyParser
      \brief Matches any character.
   */
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
