#ifndef BEAM_DECIMALPARSER_HPP
#define BEAM_DECIMALPARSER_HPP
#include <cctype>
#include <cstdlib>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class DecimalParser
      \brief Matches a decimal value.
      \tparam FloatingType The floating point data type to store the value in.
   */
  template<typename FloatingType>
  class DecimalParser : public ParserOperators {
    public:
      typedef FloatingType Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename FloatingType>
  template<typename ParserStreamType>
  bool DecimalParser<FloatingType>::Read(ParserStreamType& source,
      Result& value) {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
    } state = START;
    SubParserStream<ParserStreamType> context(source);
    char decimalBuffer[64];
    std::size_t count = 0;
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() == '-') {
      decimalBuffer[count] = '-';
      ++count;
      if(!context.Read()) {
        return false;
      }
    }
    if(std::isdigit(context.GetChar())) {
      decimalBuffer[count] = context.GetChar();
      ++count;
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
          decimalBuffer[count] = context.GetChar();
          ++count;
        } else if(context.GetChar() == '.') {
          decimalBuffer[count] = '.';
          ++count;
          state = START_DECIMAL;
        } else {
          context.Undo();
          state = TERMINAL;
          break;
        }
      } else if(state == START_DECIMAL) {
        if(std::isdigit(context.GetChar())) {
          decimalBuffer[count] = context.GetChar();
          ++count;
          state = DECIMAL_DIGITS;
        } else {
          break;
        }
      } else if(state == DECIMAL_DIGITS) {
        if(std::isdigit(context.GetChar())) {
          decimalBuffer[count] = context.GetChar();
          ++count;
        } else {
          context.Undo();
          state = TERMINAL;
          break;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.Accept();
    decimalBuffer[count] = '\0';
    value = std::strtod(decimalBuffer, nullptr);
    return true;
  }

  template<typename FloatingType>
  template<typename ParserStreamType>
  bool DecimalParser<FloatingType>::Read(ParserStreamType& source) {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
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
    while(context.Read()) {
      if(state == INTEGER_DIGITS) {
        if(std::isdigit(context.GetChar())) {
          continue;
        } else if(context.GetChar() == '.') {
          state = START_DECIMAL;
        } else {
          context.Undo();
          state = TERMINAL;
          break;
        }
      } else if(state == START_DECIMAL) {
        if(std::isdigit(context.GetChar())) {
          state = DECIMAL_DIGITS;
        } else {
          break;
        }
      } else if(state == DECIMAL_DIGITS) {
        if(std::isdigit(context.GetChar())) {
          continue;
        } else {
          context.Undo();
          state = TERMINAL;
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
}

#endif
