#ifndef BEAM_BLANKPARSER_HPP
#define BEAM_BLANKPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class BlankParser
      \brief Matches a blank character.
   */
  class BlankParser : public ParserOperators {
    public:
      typedef char Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, char& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool BlankParser::Read(ParserStreamType& source, char& value) {
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

  template<typename ParserStreamType>
  bool BlankParser::Read(ParserStreamType& source) {
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
}

#endif
