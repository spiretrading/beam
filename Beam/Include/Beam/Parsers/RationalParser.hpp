#ifndef BEAM_RATIONAL_PARSER_HPP
#define BEAM_RATIONAL_PARSER_HPP
#include <cctype>
#include <boost/rational.hpp>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Matches a rational value.
   * @tparam I The integral type used for the numerator/denominator.
   */
  template<typename I>
  class RationalParser {
    public:
      using Integral = I;
      using Result = boost::rational<Integral>;

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** The default parser for boost::rational<int>. */
  inline const auto rational_p = RationalParser<int>();

  template<typename I>
  template<IsParserStream S>
  bool RationalParser<I>::read(S& source, Result& value) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
    } state = START;
    auto context = SubParserStream<S>(source);
    auto numerator = Integral(0);
    auto denominator = Integral(0);
    auto sign = Integral();
    auto count = std::size_t(1);
    if(!context.read()) {
      return false;
    }
    if(context.peek() == '-') {
      sign = -1;
      if(!context.read()) {
        return false;
      }
    } else {
      sign = 1;
    }
    if(std::isdigit(context.peek())) {
      numerator = context.peek() - '0';
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
          numerator = 10 * numerator + (context.peek() - '0');
        } else if(context.peek() == '.') {
          state = START_DECIMAL;
        } else {
          context.undo();
          state = TERMINAL;
          break;
        }
      } else if(state == START_DECIMAL) {
        if(std::isdigit(context.peek())) {
          denominator = (context.peek() - '0');
          count = 10;
          state = DECIMAL_DIGITS;
        } else {
          break;
        }
      } else if(state == DECIMAL_DIGITS) {
        if(std::isdigit(context.peek())) {
          denominator = 10 * denominator + (context.peek() - '0');
          count *= 10;
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
    auto parsed_numerator = sign * (count * numerator + denominator);
    auto parsed_denominator = count;
    auto divisor = boost::gcd(parsed_numerator, parsed_denominator);
    value.assign(static_cast<Integral>(parsed_numerator / divisor),
      static_cast<Integral>(parsed_denominator / divisor));
    return true;
  }

  template<typename I>
  template<IsParserStream S>
  bool RationalParser<I>::read(S& source) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      END_INTEGER,
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
          context.undo();
          state = TERMINAL;
          break;
        }
      } else if(state == END_INTEGER) {
        if(context.peek() == '.') {
          state = START_DECIMAL;
        } else {
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
