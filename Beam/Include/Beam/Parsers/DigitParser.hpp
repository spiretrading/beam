#ifndef BEAM_DIGITPARSER_HPP
#define BEAM_DIGITPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class DigitParser
      \brief Matches a digit character.
   */
  class DigitParser : public ParserOperators {
    public:
      typedef char Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, char& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool DigitParser::Read(ParserStreamType& source, char& value) {
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

  template<typename ParserStreamType>
  bool DigitParser::Read(ParserStreamType& source) {
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
}

#endif
