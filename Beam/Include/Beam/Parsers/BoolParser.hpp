#ifndef BEAM_BOOL_PARSER_HPP
#define BEAM_BOOL_PARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /** Matches the boolean symbols "true" and "false". */
  class BoolParser {
    public:
      using Result = bool;

      template<IsParserStream S>
      bool read(S& source, bool& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of a BoolParser. */
  inline const auto bool_p = BoolParser();

  template<IsParserStream S>
  bool BoolParser::read(S& source, bool& value) const {
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() == 't' && context.read()) {
      if(context.peek() != 'r' || !context.read()) {
        return false;
      }
      if(context.peek() != 'u' || !context.read()) {
        return false;
      }
      if(context.peek() != 'e') {
        return false;
      }
      context.accept();
      value = true;
      return true;
    } else if(context.peek() == 'f' && context.read()) {
      if(context.peek() != 'a' || !context.read()) {
        return false;
      }
      if(context.peek() != 'l' || !context.read()) {
        return false;
      }
      if(context.peek() != 's' || !context.read()) {
        return false;
      }
      if(context.peek() != 'e') {
        return false;
      }
      context.accept();
      value = false;
      return true;
    }
    return false;
  }

  template<IsParserStream S>
  bool BoolParser::read(S& source) const {
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() == 't' && context.read()) {
      if(context.peek() != 'r' || !context.read()) {
        return false;
      }
      if(context.peek() != 'u' || !context.read()) {
        return false;
      }
      if(context.peek() != 'e') {
        return false;
      }
      context.accept();
      return true;
    } else if(context.peek() == 'f' && context.read()) {
      if(context.peek() != 'a' || !context.read()) {
        return false;
      }
      if(context.peek() != 'l' || !context.read()) {
        return false;
      }
      if(context.peek() != 's' || !context.read()) {
        return false;
      }
      if(context.peek() != 'e') {
        return false;
      }
      context.accept();
      return true;
    }
    return false;
  }
}

#endif
