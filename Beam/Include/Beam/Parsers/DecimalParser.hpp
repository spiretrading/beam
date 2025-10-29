#ifndef BEAM_DECIMAL_PARSER_HPP
#define BEAM_DECIMAL_PARSER_HPP
#include <array>
#include <cctype>
#include <cstdlib>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Matches a decimal value.
   * @tparam F The floating point data type to store the value in.
   */
  template<typename F>
  class DecimalParser {
    public:
      using Result = F;

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** A parser that matches a float value. */
  inline const auto float_p = DecimalParser<float>();

  /** A parser that matches a decimal value. */
  inline const auto double_p = DecimalParser<double>();

  template<typename F>
  template<IsParserStream S>
  bool DecimalParser<F>::read(S& source, Result& value) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
    } state = START;
    auto context = SubParserStream<S>(source);
    auto decimal_buffer = std::array<char, 64>();
    auto count = std::size_t(0);
    if(!context.read()) {
      return false;
    }
    if(context.peek() == '-') {
      decimal_buffer[count] = '-';
      ++count;
      if(!context.read()) {
        return false;
      }
    }
    if(std::isdigit(context.peek())) {
      decimal_buffer[count] = context.peek();
      ++count;
      state = INTEGER_DIGITS;
    } else {
      return false;
    }
    while(state != TERMINAL) {
      if(!context.read()) {
        state = TERMINAL;
        continue;
      }
      if(state == INTEGER_DIGITS) {
        if(std::isdigit(context.peek())) {
          decimal_buffer[count] = context.peek();
          ++count;
        } else if(context.peek() == '.') {
          decimal_buffer[count] = '.';
          ++count;
          state = START_DECIMAL;
        } else {
          context.undo();
          state = TERMINAL;
          break;
        }
      } else if(state == START_DECIMAL) {
        if(std::isdigit(context.peek())) {
          decimal_buffer[count] = context.peek();
          ++count;
          state = DECIMAL_DIGITS;
        } else {
          break;
        }
      } else if(state == DECIMAL_DIGITS) {
        if(std::isdigit(context.peek())) {
          decimal_buffer[count] = context.peek();
          ++count;
        } else {
          context.undo();
          state = TERMINAL;
          break;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.accept();
    decimal_buffer[count] = '\0';
    value = std::strtod(decimal_buffer.data(), nullptr);
    return true;
  }

  template<typename F>
  template<IsParserStream S>
  bool DecimalParser<F>::read(S& source) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
    } state = START;
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() == '-') {
      if(!context.read()) {
        return false;
      }
    }
    if(std::isdigit(context.peek())) {
      state = INTEGER_DIGITS;
    } else {
      return false;
    }
    while(context.read()) {
      if(state == INTEGER_DIGITS) {
        if(std::isdigit(context.peek())) {
          continue;
        } else if(context.peek() == '.') {
          state = START_DECIMAL;
        } else {
          context.Undo();
          state = TERMINAL;
          break;
        }
      } else if(state == START_DECIMAL) {
        if(std::isdigit(context.peek())) {
          state = DECIMAL_DIGITS;
        } else {
          break;
        }
      } else if(state == DECIMAL_DIGITS) {
        if(std::isdigit(context.peek())) {
          continue;
        } else {
          context.undo();
          state = TERMINAL;
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
