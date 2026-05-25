#ifndef BEAM_HTML_UTILITIES_HPP
#define BEAM_HTML_UTILITIES_HPP
#include <string>
#include <string_view>

namespace Beam {

  /**
   * Escapes a string for safe inclusion in HTML content.
   * @param source The string to escape.
   * @return The HTML-escaped string.
   */
  inline std::string escape_html(std::string_view source) {
    auto result = std::string();
    result.reserve(source.size());
    for(auto c : source) {
      switch(c) {
        case '&':
          result += "&amp;";
          break;
        case '<':
          result += "&lt;";
          break;
        case '>':
          result += "&gt;";
          break;
        case '"':
          result += "&quot;";
          break;
        case '\'':
          result += "&#39;";
          break;
        default:
          result += c;
      }
    }
    return result;
  }

  /**
   * Unescapes HTML entities back to their literal characters.
   * @param source The HTML-escaped string.
   * @return The unescaped string.
   */
  inline std::string unescape_html(std::string_view source) {
    auto result = std::string();
    result.reserve(source.size());
    for(auto i = std::size_t(0); i < source.size(); ++i) {
      if(source[i] == '&') {
        if(source.substr(i, 5) == "&amp;") {
          result += '&';
          i += 4;
        } else if(source.substr(i, 4) == "&lt;") {
          result += '<';
          i += 3;
        } else if(source.substr(i, 4) == "&gt;") {
          result += '>';
          i += 3;
        } else if(source.substr(i, 6) == "&quot;") {
          result += '"';
          i += 5;
        } else if(source.substr(i, 5) == "&#39;") {
          result += '\'';
          i += 4;
        } else {
          result += '&';
        }
      } else {
        result += source[i];
      }
    }
    return result;
  }
}

#endif
