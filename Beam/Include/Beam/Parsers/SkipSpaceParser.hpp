#ifndef BEAM_SKIPSPACEPARSER_HPP
#define BEAM_SKIPSPACEPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam::Parsers {

  /*! \class SkipSpaceParser
      \brief Parser used to skip spaces.
   */
  class SkipSpaceParser {
    public:
      using Result = NullType;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool SkipSpaceParser::Read(Stream& source) const {
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

#endif
