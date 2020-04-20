#ifndef BEAM_RATIONALPARSER_HPP
#define BEAM_RATIONALPARSER_HPP
#include <cctype>
#include <boost/rational.hpp>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam::Parsers {

  /*! \class RationalParser
      \brief Matches a rational value.
      \tparam I The integral type used for the numerator/denominator.
   */
  template<typename I>
  class RationalParser {
    public:
      using Integral = I;
      using Result = boost::rational<Integral>;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename I>
  template<typename Stream>
  bool RationalParser<I>::Read(Stream& source, Result& value) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      START_DECIMAL,
      DECIMAL_DIGITS
    } state = START;
    auto context = SubParserStream<Stream>(source);
    auto numerator = Integral(0);
    auto denominator = Integral(0);
    auto sign = Integral();
    auto count = std::size_t(1);
    if(!context.Read()) {
      return false;
    }
    if(context.GetChar() == '-') {
      sign = -1;
      if(!context.Read()) {
        return false;
      }
    } else {
      sign = 1;
    }
    if(std::isdigit(context.GetChar())) {
      numerator = context.GetChar() - '0';
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
          numerator = 10 * numerator + (context.GetChar() - '0');
        } else if(context.GetChar() == '.') {
          state = START_DECIMAL;
        } else {
          context.Undo();
          state = TERMINAL;
          break;
        }
      } else if(state == START_DECIMAL) {
        if(std::isdigit(context.GetChar())) {
          denominator = (context.GetChar() - '0');
          count = 10;
          state = DECIMAL_DIGITS;
        } else {
          break;
        }
      } else if(state == DECIMAL_DIGITS) {
        if(std::isdigit(context.GetChar())) {
          denominator = 10 * denominator + (context.GetChar() - '0');
          count *= 10;
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
    auto parsedNumerator = sign * (count * numerator + denominator);
    auto parsedDenominator = count;
    auto divisor = boost::gcd(parsedNumerator, parsedDenominator);
    value.assign(static_cast<Integral>(parsedNumerator / divisor),
      static_cast<Integral>(parsedDenominator / divisor));
    return true;
  }

  template<typename I>
  template<typename Stream>
  bool RationalParser<I>::Read(Stream& source) const {
    enum {
      START,
      TERMINAL,
      INTEGER_DIGITS,
      END_INTEGER,
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
      } else if(state == END_INTEGER) {
        if(context.GetChar() == '.') {
          state = START_DECIMAL;
        } else {
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
