#ifndef BEAM_ANYPARSER_HPP
#define BEAM_ANYPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class AnyParser
      \brief Matches any character.
   */
  class AnyParser : public ParserOperators {
    public:
      typedef char Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, char& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool AnyParser::Read(ParserStreamType& source, char& value) {
    if(!source.Read()) {
      return false;
    }
    value = source.GetChar();
    return true;
  }

  template<typename ParserStreamType>
  bool AnyParser::Read(ParserStreamType& source) {
    return source.Read();
  }
}
}

#endif
