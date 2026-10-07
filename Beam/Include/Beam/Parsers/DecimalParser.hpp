#ifndef BEAM_DECIMAL_PARSER_HPP
#define BEAM_DECIMAL_PARSER_HPP
#include <charconv>
#include <string>
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
    auto context = SubParserStream<S>(source);
    auto buffer = std::string();
    auto read_digits = [&] {
      auto size = buffer.size();
      while(context.read()) {
        auto character = context.peek();
        if(character < '0' || character > '9') {
          context.undo();
          break;
        }
        buffer += character;
      }
      return buffer.size() != size;
    };
    if(!context.read()) {
      return false;
    }
    if(context.peek() == '-') {
      buffer += '-';
    } else {
      context.undo();
    }
    if(!read_digits()) {
      return false;
    }
    if(context.read()) {
      if(context.peek() == '.') {
        buffer += '.';
        if(!read_digits()) {
          return false;
        }
      } else {
        context.undo();
      }
    }
    if(context.read()) {
      if(context.peek() == 'e' || context.peek() == 'E') {
        buffer += context.peek();
        if(context.read()) {
          if(context.peek() == '+' || context.peek() == '-') {
            buffer += context.peek();
          } else {
            context.undo();
          }
        }
        if(!read_digits()) {
          return false;
        }
      } else {
        context.undo();
      }
    }
    auto result = Result();
    auto end = buffer.data() + buffer.size();
    auto conversion = std::from_chars(buffer.data(), end, result);
    if(conversion.ec != std::errc() || conversion.ptr != end) {
      return false;
    }
    value = result;
    context.accept();
    return true;
  }

  template<typename F>
  template<IsParserStream S>
  bool DecimalParser<F>::read(S& source) const {
    auto value = Result();
    return read(source, value);
  }
}

#endif
