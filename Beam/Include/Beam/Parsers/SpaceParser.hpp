#ifndef BEAM_SPACEPARSER_HPP
#define BEAM_SPACEPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class SpaceParser
      \brief Matches a blank character.
   */
  class SpaceParser : public ParserOperators {
    public:
      typedef char Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, char& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool SpaceParser::Read(ParserStreamType& source, char& value) {
    if(!source.Read()) {
      return false;
    }
    value = source.GetChar();
    if(std::isspace(value)) {
      return true;
    }
    source.Undo();
    return false;
  }

  template<typename ParserStreamType>
  bool SpaceParser::Read(ParserStreamType& source) {
    if(!source.Read()) {
      return false;
    }
    if(std::isspace(source.GetChar())) {
      return true;
    }
    source.Undo();
    return false;
  }
}
}

#endif
