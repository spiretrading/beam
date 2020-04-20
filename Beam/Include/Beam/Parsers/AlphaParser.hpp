#ifndef BEAM_ALPHAPARSER_HPP
#define BEAM_ALPHAPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /*! \class AlphaParser
      \brief Matches an alphabet character.
   */
  class AlphaParser {
    public:
      using Result = char;

      template<typename Stream>
      bool Read(Stream& source, char& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool AlphaParser::Read(Stream& source, char& value) const {
    if(!source.Read()) {
      return false;
    }
    value = source.GetChar();
    if(std::isalpha(value)) {
      return true;
    }
    source.Undo();
    return false;
  }

  template<typename Stream>
  bool AlphaParser::Read(Stream& source) const {
    if(!source.Read()) {
      return false;
    }
    if(std::isalpha(source.GetChar())) {
      return true;
    }
    source.Undo();
    return false;
  }
}

#endif
