#ifndef BEAM_DECIMALPARSER_HPP
#define BEAM_DECIMALPARSER_HPP
#include <array>
#include <cctype>
#include <cstdlib>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class DecimalParser
      \brief Matches a decimal value.
      \tparam F The floating point data type to store the value in.
   */
  template<typename F>
  class DecimalParser {
    public:
      using Result = F;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename F>
  template<typename Stream>
  bool DecimalParser<F>::Read(Stream& source, Result& value) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
    } state = START;
    auto context = SubParserStream<Stream>(source);
    auto decimalBuffer = std::array<char, 64>();
    auto count = std::size_t(0);
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
    value = std::strtod(decimalBuffer.data(), nullptr);
    return true;
  }

  template<typename F>
  template<typename Stream>
  bool DecimalParser<F>::Read(Stream& source) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
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

#endif
