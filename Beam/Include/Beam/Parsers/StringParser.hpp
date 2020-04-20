#ifndef BEAM_STRINGPARSER_HPP
#define BEAM_STRINGPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class StringParser
      \brief Matches a quoted string.
   */
  class StringParser {
    public:
      using Result = std::string;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool StringParser::Read(Stream& source, Result& value) const {
    value.clear();
    auto context = SubParserStream<Stream>(source);
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() != '\"') {
      return false;
    }
    enum {
      NORMAL,
      TERMINAL,
      ESCAPE
    } state = NORMAL;
    while(context.Read()) {
      if(state == NORMAL) {
        if(context.GetChar() == '\"') {
          state = TERMINAL;
          break;
        } else if(context.GetChar() == '\\') {
          state = ESCAPE;
        } else if(!std::iscntrl(context.GetChar())) {
          value += context.GetChar();
        } else {
          break;
        }
      } else if(state == ESCAPE) {
        if(context.GetChar() == 'n') {
          value += '\n';
          state = NORMAL;
        } else if(context.GetChar() == 't') {
          value += '\t';
          state = NORMAL;
        } else if(context.GetChar() == '\"') {
          value += '\"';
          state = NORMAL;
        } else if(context.GetChar() == '\\') {
          value += '\\';
          state = NORMAL;
        } else if(context.GetChar() == '/') {
          value += '/';
          state = NORMAL;
        } else if(context.GetChar() == 'b') {
          value += '\b';
          state = NORMAL;
        } else if(context.GetChar() == 'f') {
          value += '\f';
          state = NORMAL;
        } else if(context.GetChar() == 'r') {
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
    context.Accept();
    return true;
  }

  template<typename Stream>
  bool StringParser::Read(Stream& source) const {
    auto context = SubParserStream<Stream>(source);
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() != '\"') {
      return false;
    }
    enum {
      NORMAL,
      TERMINAL,
      ESCAPE
    } state = NORMAL;
    while(context.Read()) {
      if(state == NORMAL) {
        if(context.GetChar() == '\"') {
          state = TERMINAL;
          break;
        } else if(context.GetChar() == '\\') {
          state = ESCAPE;
        } else if(!std::iscntrl(context.GetChar())) {
          continue;
        } else {
          break;
        }
      } else if(state == ESCAPE) {
        if(context.GetChar() == 'n') {
          state = NORMAL;
        } else if(context.GetChar() == 't') {
          state = NORMAL;
        } else if(context.GetChar() == '\"') {
          state = NORMAL;
        } else if(context.GetChar() == '\\') {
          state = NORMAL;
        } else if(context.GetChar() == '/') {
          state = NORMAL;
        } else if(context.GetChar() == 'b') {
          state = NORMAL;
        } else if(context.GetChar() == 'f') {
          state = NORMAL;
        } else if(context.GetChar() == 'r') {
          state = NORMAL;
        } else {
          break;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.Accept();
    return true;
  }
}

#endif
