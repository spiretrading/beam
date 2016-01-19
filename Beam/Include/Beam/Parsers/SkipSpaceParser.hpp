#ifndef BEAM_SKIPSPACEPARSER_HPP
#define BEAM_SKIPSPACEPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class SkipSpaceParser
      \brief Parser used to skip spaces.
   */
  class SkipSpaceParser : public ParserOperators {
    public:
      typedef NullType Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool SkipSpaceParser::Read(ParserStreamType& source) {
    while(true) {
      if(!source.Read()) {
        return true;
      }
      if(!std::isspace(source.GetChar())) {
        source.Undo();
        return true;
      }
    }
    return true;
  }
}
}

#endif
