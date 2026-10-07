#ifndef BEAM_DECIMAL_PARSER_HPP
#define BEAM_DECIMAL_PARSER_HPP
#include <charconv>
#include <string>
#include <string_view>
#include <type_traits>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Converts a decimal token into a numeric value. Specialize this
   * function for types requiring a custom decimal conversion.
   * @tparam F The numeric data type to store the value in.
   * @param text A syntactically valid decimal token.
   * @param value Stores the converted value, unchanged on failure.
   * @return True if the entire string was converted successfully.
   */
  template<typename F>
  bool parse_decimal(std::string_view text, F& value) {
    using Value = std::conditional_t<std::is_floating_point_v<F>, F, double>;
    auto result = Value();
    auto end = text.data() + text.size();
    auto conversion = std::from_chars(text.data(), end, result);
    if(conversion.ec != std::errc() || conversion.ptr != end) {
      return false;
    }
    value = static_cast<F>(result);
    return true;
  }

  /**
   * Matches a decimal value.
   * @tparam F The numeric data type to store the value in.
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
    if(!parse_decimal(buffer, value)) {
      return false;
    }
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
