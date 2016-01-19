#ifndef BEAM_INTEGRALPARSER_HPP
#define BEAM_INTEGRALPARSER_HPP
#include <cctype>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class IntegralParser
      \brief Matches an integral value.
      \tparam IntegralType The type of integral value to parse.
   */
  template<typename IntegralType>
  class IntegralParser : public ParserOperators {
    public:
      typedef IntegralType Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename IntegralType>
  template<typename ParserStreamType>
  bool IntegralParser<IntegralType>::Read(ParserStreamType& source,
      Result& value) {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
    } state = START;
    SubParserStream<ParserStreamType> context(source);
    if(!context.Read()) {
      return false;
    }
    Result sign;
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

  template<typename IntegralType>
  template<typename ParserStreamType>
  bool IntegralParser<IntegralType>::Read(ParserStreamType& source) {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
    } state = START;
    SubParserStream<ParserStreamType> context(source);
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
}

#endif
