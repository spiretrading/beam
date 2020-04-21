#ifndef BEAM_BOOLPARSER_HPP
#define BEAM_BOOLPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class BoolParser
      \brief Matches a bool symbol.
   */
  class BoolParser {
    public:
      using Result = bool;

      template<typename Stream>
      bool Read(Stream& source, bool& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool BoolParser::Read(Stream& source, bool& value) const {
    auto context = SubParserStream<Stream>(source);
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() == 't' && context.Read()) {
      if(context.GetChar() != 'r' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'u' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'e') {
        return false;
      }
      context.Accept();
      value = true;
      return true;
    } else if(context.GetChar() == 'f' && context.Read()) {
      if(context.GetChar() != 'a' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'l' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 's' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'e') {
        return false;
      }
      context.Accept();
      value = false;
      return true;
    }
    return false;
  }

  template<typename Stream>
  bool BoolParser::Read(Stream& source) const {
    auto context = SubParserStream<Stream>(source);
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() == 't' && context.Read()) {
      if(context.GetChar() != 'r' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'u' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'e') {
        return false;
      }
      context.Accept();
      return true;
    } else if(context.GetChar() == 'f' && context.Read()) {
      if(context.GetChar() != 'a' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'l' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 's' || !context.Read()) {
        return false;
      }
      if(context.GetChar() != 'e') {
        return false;
      }
      context.Accept();
      return true;
    }
    return false;
  }
}

#endif
