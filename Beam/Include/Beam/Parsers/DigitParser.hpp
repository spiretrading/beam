#ifndef BEAM_DIGITPARSER_HPP
#define BEAM_DIGITPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /*! \class DigitParser
      \brief Matches a digit character.
   */
  class DigitParser {
    public:
      using Result = char;

      template<typename Stream>
      bool Read(Stream& source, char& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool DigitParser::Read(Stream& source, char& value) const {
    if(!source.Read()) {
      return false;
    }
    value = source.GetChar();
    if(std::isdigit(value)) {
      return true;
    }
    source.Undo();
    return false;
  }

  template<typename Stream>
  bool DigitParser::Read(Stream& source) const {
    if(!source.Read()) {
      return false;
    }
    if(std::isdigit(source.GetChar())) {
      return true;
    }
    source.Undo();
    return false;
  }
}

#endif
