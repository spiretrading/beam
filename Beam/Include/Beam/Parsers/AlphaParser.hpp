#ifndef BEAM_ALPHAPARSER_HPP
#define BEAM_ALPHAPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class AlphaParser
      \brief Matches an alphabet character.
   */
  class AlphaParser : public ParserOperators {
    public:
      typedef char Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, char& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool AlphaParser::Read(ParserStreamType& source, char& value) {
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

  template<typename ParserStreamType>
  bool AlphaParser::Read(ParserStreamType& source) {
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
}

#endif
