#ifndef BEAM_SPACEPARSER_HPP
#define BEAM_SPACEPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /*! \class SpaceParser
      \brief Matches a blank character.
   */
  class SpaceParser {
    public:
      using Result = char;

      template<typename Stream>
      bool Read(Stream& source, char& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool SpaceParser::Read(Stream& source, char& value) const {
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

  template<typename Stream>
  bool SpaceParser::Read(Stream& source) const {
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

#endif
