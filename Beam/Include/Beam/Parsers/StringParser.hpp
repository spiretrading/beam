#ifndef BEAM_STRING_PARSER_HPP
#define BEAM_STRING_PARSER_HPP
#include <cstdint>
#include <string>
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {
namespace Details {
  template<IsParserStream S>
  bool read_hex_escape(S& source, std::uint32_t& value) {
    value = 0;
    for(auto i = 0; i != 4; ++i) {
      if(!source.read()) {
        return false;
      }
      auto character = source.peek();
      auto digit = std::uint32_t();
      if(character >= '0' && character <= '9') {
        digit = character - '0';
      } else if(character >= 'a' && character <= 'f') {
        digit = character - 'a' + 10;
      } else if(character >= 'A' && character <= 'F') {
        digit = character - 'A' + 10;
      } else {
        return false;
      }
      value = (value << 4) | digit;
    }
    return true;
  }

  template<IsParserStream S>
  bool read_unicode_escape(S& source, std::uint32_t& value) {
    if(!read_hex_escape(source, value)) {
      return false;
    }
    if(value >= 0xdc00 && value <= 0xdfff) {
      return false;
    }
    if(value >= 0xd800 && value <= 0xdbff) {
      if(!source.read() || source.peek() != '\\' ||
          !source.read() || source.peek() != 'u') {
        return false;
      }
      auto low = std::uint32_t();
      if(!read_hex_escape(source, low) || low < 0xdc00 || low > 0xdfff) {
        return false;
      }
      value = 0x10000 + ((value - 0xd800) << 10) + low - 0xdc00;
    }
    return true;
  }
}

  /** Matches a quoted string. */
  class StringParser {
    public:
      using Result = std::string;

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;
  };

  /** The global instance of a StringParser. */
  inline const auto string_p = StringParser();

  template<IsParserStream S>
  bool StringParser::read(S& source, Result& value) const {
    value.clear();
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() != '\"') {
      return false;
    }
    enum {
      NORMAL,
      TERMINAL,
      ESCAPE
    } state = NORMAL;
    while(context.read()) {
      if(state == NORMAL) {
        if(context.peek() == '\"') {
          state = TERMINAL;
          break;
        } else if(context.peek() == '\\') {
          state = ESCAPE;
        } else if(static_cast<unsigned char>(context.peek()) >= 0x20) {
          value += context.peek();
        } else {
          break;
        }
      } else if(state == ESCAPE) {
        if(context.peek() == 'n') {
          value += '\n';
          state = NORMAL;
        } else if(context.peek() == 't') {
          value += '\t';
          state = NORMAL;
        } else if(context.peek() == '\"') {
          value += '\"';
          state = NORMAL;
        } else if(context.peek() == '\\') {
          value += '\\';
          state = NORMAL;
        } else if(context.peek() == '/') {
          value += '/';
          state = NORMAL;
        } else if(context.peek() == 'b') {
          value += '\b';
          state = NORMAL;
        } else if(context.peek() == 'f') {
          value += '\f';
          state = NORMAL;
        } else if(context.peek() == 'r') {
          value += '\r';
          state = NORMAL;
        } else if(context.peek() == 'u') {
          auto point = std::uint32_t();
          if(!Details::read_unicode_escape(context, point)) {
            break;
          }
          if(point <= 0x7f) {
            value += static_cast<char>(point);
          } else if(point <= 0x7ff) {
            value += static_cast<char>(0xc0 | (point >> 6));
            value += static_cast<char>(0x80 | (point & 0x3f));
          } else if(point <= 0xffff) {
            value += static_cast<char>(0xe0 | (point >> 12));
            value += static_cast<char>(0x80 | ((point >> 6) & 0x3f));
            value += static_cast<char>(0x80 | (point & 0x3f));
          } else {
            value += static_cast<char>(0xf0 | (point >> 18));
            value += static_cast<char>(0x80 | ((point >> 12) & 0x3f));
            value += static_cast<char>(0x80 | ((point >> 6) & 0x3f));
            value += static_cast<char>(0x80 | (point & 0x3f));
          }
          state = NORMAL;
        } else {
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

  template<IsParserStream S>
  bool StringParser::read(S& source) const {
    auto context = SubParserStream<S>(source);
    if(!context.read()) {
      return false;
    }
    if(context.peek() != '\"') {
      return false;
    }
    enum {
      NORMAL,
      TERMINAL,
      ESCAPE
    } state = NORMAL;
    while(context.read()) {
      if(state == NORMAL) {
        if(context.peek() == '\"') {
          state = TERMINAL;
          break;
        } else if(context.peek() == '\\') {
          state = ESCAPE;
        } else if(static_cast<unsigned char>(context.peek()) >= 0x20) {
          continue;
        } else {
          break;
        }
      } else if(state == ESCAPE) {
        if(context.peek() == 'n') {
          state = NORMAL;
        } else if(context.peek() == 't') {
          state = NORMAL;
        } else if(context.peek() == '\"') {
          state = NORMAL;
        } else if(context.peek() == '\\') {
          state = NORMAL;
        } else if(context.peek() == '/') {
          state = NORMAL;
        } else if(context.peek() == 'b') {
          state = NORMAL;
        } else if(context.peek() == 'f') {
          state = NORMAL;
        } else if(context.peek() == 'r') {
          state = NORMAL;
        } else if(context.peek() == 'u') {
          auto point = std::uint32_t();
          if(!Details::read_unicode_escape(context, point)) {
            break;
          }
          state = NORMAL;
        } else {
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
