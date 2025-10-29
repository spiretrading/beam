#ifndef BEAM_INTEGRAL_PARSER_HPP
#define BEAM_INTEGRAL_PARSER_HPP
#include <cctype>
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Matches an integral value.
   * @tparam I The type of integral value to parse.
   */
  template<typename I>
  class IntegralParser {
    public:
      using Result = I;

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** An instance of an IntegralParser that parses int values. */
  inline const auto int_p = IntegralParser<int>();

  /** An instance of an IntegralParser that parses int64_t values. */
  inline const auto int64_p = IntegralParser<std::int64_t>();

  template<typename I>
  template<IsParserStream S>
  bool IntegralParser<I>::read(S& source, Result& value) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
    } state = START;
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    auto sign = Result();
    if(context.peek() == '-') {
      sign = -1;
      if(!context.read()) {
        return false;
      }
    } else {
      sign = 1;
    }
    if(std::isdigit(context.peek())) {
      value = context.peek() - '0';
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
          value = 10 * value + (context.peek() - '0');
        } else {
          context.undo();
          state = TERMINAL;
        }
      }
    }
    if(state != TERMINAL) {
      return false;
    }
    context.accept();
    value *= sign;
    return true;
  }

  template<typename I>
  template<IsParserStream S>
  bool IntegralParser<I>::read(S& source) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
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
    while(state != TERMINAL) {
      if(!context.read()) {
        state = TERMINAL;
        continue;
      }
      if(state == INTEGER_DIGITS) {
        if(!std::isdigit(context.peek())) {
          context.undo();
          state = TERMINAL;
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
