#ifndef BEAM_BOOLPARSER_HPP
#define BEAM_BOOLPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class BoolParser
      \brief Matches a bool symbol.
   */
  class BoolParser : public ParserOperators {
    public:
      typedef bool Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, bool& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  bool BoolParser::Read(ParserStreamType& source, bool& value) {
    SubParserStream<ParserStreamType> context(source);
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

  template<typename ParserStreamType>
  bool BoolParser::Read(ParserStreamType& source) {
    SubParserStream<ParserStreamType> context(source);
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
}

#endif
