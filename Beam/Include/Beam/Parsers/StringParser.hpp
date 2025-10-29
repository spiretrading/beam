#ifndef BEAM_STRING_PARSER_HPP
#define BEAM_STRING_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /** Matches a quoted string. */
  class StringParser {
    public:
      using Result = std::string;

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** The global instance of a StringParser. */
  inline const auto string_p = StringParser();

  template<IsParserStream S>
  bool StringParser::read(S& source, Result& value) const {
    value.clear();
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() != '\"') {
      return false;
    }
    enum {
      NORMAL,
      TERMINAL,
      ESCAPE
    } state = NORMAL;
    while(context.read()) {
      if(state == NORMAL) {
        if(context.peek() == '\"') {
          state = TERMINAL;
          break;
        } else if(context.peek() == '\\') {
          state = ESCAPE;
        } else if(!std::iscntrl(context.peek())) {
          value += context.peek();
        } else {
          break;
        }
      } else if(state == ESCAPE) {
        if(context.peek() == 'n') {
          value += '\n';
          state = NORMAL;
        } else if(context.peek() == 't') {
          value += '\t';
          state = NORMAL;
        } else if(context.peek() == '\"') {
          value += '\"';
          state = NORMAL;
        } else if(context.peek() == '\\') {
          value += '\\';
          state = NORMAL;
        } else if(context.peek() == '/') {
          value += '/';
          state = NORMAL;
        } else if(context.peek() == 'b') {
          value += '\b';
          state = NORMAL;
        } else if(context.peek() == 'f') {
          value += '\f';
          state = NORMAL;
        } else if(context.peek() == 'r') {
          value += '\r';
          state = NORMAL;
        } else {
          break;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.accept();
    return true;
  }

  template<IsParserStream S>
  bool StringParser::read(S& source) const {
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() != '\"') {
      return false;
    }
    enum {
      NORMAL,
      TERMINAL,
      ESCAPE
    } state = NORMAL;
    while(context.read()) {
      if(state == NORMAL) {
        if(context.peek() == '\"') {
          state = TERMINAL;
          break;
        } else if(context.peek() == '\\') {
          state = ESCAPE;
        } else if(!std::iscntrl(context.peek())) {
          continue;
        } else {
          break;
        }
      } else if(state == ESCAPE) {
        if(context.peek() == 'n') {
          state = NORMAL;
        } else if(context.peek() == 't') {
          state = NORMAL;
        } else if(context.peek() == '\"') {
          state = NORMAL;
        } else if(context.peek() == '\\') {
          state = NORMAL;
        } else if(context.peek() == '/') {
          state = NORMAL;
        } else if(context.peek() == 'b') {
          state = NORMAL;
        } else if(context.peek() == 'f') {
          state = NORMAL;
        } else if(context.peek() == 'r') {
          state = NORMAL;
        } else {
          break;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.accept();
    return true;
  }
}

#endif
