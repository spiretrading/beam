#ifndef BEAM_INTEGRALPARSER_HPP
#define BEAM_INTEGRALPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class IntegralParser
      \brief Matches an integral value.
      \tparam I The type of integral value to parse.
   */
  template<typename I>
  class IntegralParser {
    public:
      using Result = I;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename I>
  template<typename Stream>
  bool IntegralParser<I>::Read(Stream& source, Result& value) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
    } state = START;
    auto context = SubParserStream<Stream>(source);
    if(!context.Read()) {
      return false;
    }
    auto sign = Result();
    if(context.GetChar() == '-') {
      sign = -1;
      if(!context.Read()) {
        return false;
      }
    } else {
      sign = 1;
    }
    if(std::isdigit(context.GetChar())) {
      value = context.GetChar() - '0';
      state = INTEGER_DIGITS;
    } else {
      return false;
    }
    while(state != TERMINAL) {
      if(!context.Read()) {
        state = TERMINAL;
        continue;
      }
      if(state == INTEGER_DIGITS) {
        if(std::isdigit(context.GetChar())) {
          value = 10 * value + (context.GetChar() - '0');
        } else {
          context.Undo();
          state = TERMINAL;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.Accept();
    value *= sign;
    return true;
  }

  template<typename I>
  template<typename Stream>
  bool IntegralParser<I>::Read(Stream& source) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
    } state = START;
    auto context = SubParserStream<Stream>(source);
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() == '-') {
      if(!context.Read()) {
        return false;
      }
    }
    if(std::isdigit(context.GetChar())) {
      state = INTEGER_DIGITS;
    } else {
      return false;
    }
    while(state != TERMINAL) {
      if(!context.Read()) {
        state = TERMINAL;
        continue;
      }
      if(state == INTEGER_DIGITS) {
        if(!std::isdigit(context.GetChar())) {
          context.Undo();
          state = TERMINAL;
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
