#ifndef BEAM_HTTP_RESPONSE_PARSER_HPP
#define BEAM_HTTP_RESPONSE_PARSER_HPP
#include <cstdlib>
#include <cstring>
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpStatusCode.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/InvalidHttpResponseException.hpp"
#include "Beam/WebServices/TransferEncoding.hpp"

namespace Beam {

  /** Parses an HTTP response. */
  class HttpResponseParser {
    public:

      /** Constructs an HttpResponseParser. */
      HttpResponseParser() noexcept;

      /**
       * Feeds the parser additional characters to parse.
       * @param source The characters to feed to the parser.
       */
      void feed(std::string_view source);

      /** Returns the next HttpResponse. */
      boost::optional<HttpResponse> get_next_response();

      /** Returns the remaining unparsed Buffer. */
      SharedBuffer get_remaining_buffer() const;

    private:
      enum class ParserState {
        STATUS_LINE,
        HEADERS,
        BODY,
        CHUNKED_SIZE,
        CHUNKED_DATA,
        CHUNKED_END,
        ERR
      };
      ParserState m_state;
      std::string m_buffer;
      HttpVersion m_version;
      HttpStatusCode m_status_code;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      boost::optional<std::size_t> m_content_length;
      TransferEncoding m_transfer_encoding;
      std::size_t m_chunk_size;
      std::size_t m_chunk_bytes_read;
      std::size_t m_body_bytes_read;
      SharedBuffer m_body;
      std::deque<HttpResponse> m_responses;

      HttpResponseParser(const HttpResponseParser&) = delete;
      HttpResponseParser& operator =(const HttpResponseParser&) = delete;
      void parse();
      void parse_status_line();
      void parse_headers();
      void parse_body();
      void parse_chunked_size();
      void parse_chunked_data();
      void parse_chunked_end();
      void parse_cookie(std::string_view source);
      void finalize_response();
      void reset_parser();
      std::size_t find_line_end() const;
  };

  inline HttpResponseParser::HttpResponseParser() noexcept
    : m_state(ParserState::STATUS_LINE),
      m_transfer_encoding(TransferEncoding::NONE),
      m_chunk_size(0),
      m_chunk_bytes_read(0),
      m_body_bytes_read(0) {}

  inline void HttpResponseParser::feed(std::string_view source) {
    m_buffer.append(source);
    parse();
  }

  inline boost::optional<HttpResponse>
      HttpResponseParser::get_next_response() {
    if(!m_responses.empty()) {
      auto response = std::move(m_responses.front());
      m_responses.pop_front();
      return response;
    }
    if(m_state == ParserState::ERR) {
      boost::throw_with_location(InvalidHttpResponseException());
    }
    return boost::none;
  }

  inline SharedBuffer HttpResponseParser::get_remaining_buffer() const {
    return SharedBuffer(m_buffer.data(), m_buffer.size());
  }

  inline void HttpResponseParser::parse() {
    while(true) {
      if(m_state == ParserState::STATUS_LINE) {
        parse_status_line();
        if(m_state != ParserState::HEADERS) {
          return;
        }
      }
      if(m_state == ParserState::HEADERS) {
        parse_headers();
        if(m_state != ParserState::BODY &&
            m_state != ParserState::CHUNKED_SIZE) {
          return;
        }
      }
      if(m_state == ParserState::BODY) {
        parse_body();
        if(m_state != ParserState::STATUS_LINE) {
          return;
        }
      }
      if(m_state == ParserState::CHUNKED_SIZE) {
        parse_chunked_size();
        if(m_state != ParserState::CHUNKED_DATA &&
            m_state != ParserState::CHUNKED_END) {
          return;
        }
      }
      if(m_state == ParserState::CHUNKED_DATA) {
        parse_chunked_data();
        if(m_state != ParserState::CHUNKED_SIZE) {
          return;
        }
      }
      if(m_state == ParserState::CHUNKED_END) {
        parse_chunked_end();
        if(m_state != ParserState::STATUS_LINE) {
          return;
        }
      }
      if(m_state == ParserState::ERR) {
        return;
      }
    }
  }

  inline void HttpResponseParser::parse_status_line() {
    auto line_end = find_line_end();
    if(line_end == std::string::npos) {
      return;
    }
    auto line = std::string_view(m_buffer.data(), line_end);
    if(line.size() < 12) {
      m_state = ParserState::ERR;
      return;
    }
    auto version_text = line.substr(0, 8);
    if(version_text == "HTTP/1.0") {
      m_version = HttpVersion::version_1_0();
    } else if(version_text == "HTTP/1.1") {
      m_version = HttpVersion::version_1_1();
    } else {
      m_state = ParserState::ERR;
      return;
    }
    if(line[8] != ' ') {
      m_state = ParserState::ERR;
      return;
    }
    auto status_start = std::size_t(9);
    auto status_end = line.find(' ', status_start);
    if(status_end == std::string_view::npos) {
      status_end = line.size();
    }
    auto status_text = line.substr(status_start, status_end - status_start);
    try {
      auto status_value = std::stoi(std::string(status_text));
      m_status_code = static_cast<HttpStatusCode>(status_value);
    } catch(...) {
      m_state = ParserState::ERR;
      return;
    }
    m_buffer.erase(0, line_end + 2);
    m_state = ParserState::HEADERS;
  }

  inline void HttpResponseParser::parse_headers() {
    while(true) {
      auto line_end = find_line_end();
      if(line_end == std::string::npos) {
        return;
      }
      if(line_end == 0) {
        if(m_buffer.size() < 2) {
          return;
        }
        m_buffer.erase(0, 2);
        if(m_buffer.size() >= 2 && m_buffer[0] == '\r' &&
            m_buffer[1] == '\n') {
          m_buffer.erase(0, 2);
        }
        if(m_transfer_encoding == TransferEncoding::CHUNKED) {
          m_state = ParserState::CHUNKED_SIZE;
          m_chunk_size = 0;
          m_chunk_bytes_read = 0;
        } else if(m_content_length.is_initialized()) {
          m_state = ParserState::BODY;
          m_body_bytes_read = 0;
        } else {
          finalize_response();
        }
        return;
      }
      auto line = std::string_view(m_buffer.data(), line_end);
      auto colon_position = line.find(':');
      if(colon_position == std::string_view::npos) {
        m_state = ParserState::ERR;
        return;
      }
      auto name = std::string(line.substr(0, colon_position));
      auto value_start = colon_position + 1;
      if(value_start >= line.size() || line[value_start] != ' ') {
        m_state = ParserState::ERR;
        return;
      }
      ++value_start;
      auto value = std::string(line.substr(value_start));
      if(boost::iequals(name, "Content-Length")) {
        if(!m_content_length.is_initialized()) {
          try {
            m_content_length = std::stoull(value);
          } catch(...) {
            m_state = ParserState::ERR;
            return;
          }
        }
      } else if(boost::iequals(name, "Transfer-Encoding")) {
        if(boost::iequals(value, "chunked")) {
          m_transfer_encoding = TransferEncoding::CHUNKED;
        }
        m_headers.emplace_back(std::move(name), std::move(value));
      } else if(boost::iequals(name, "Set-Cookie")) {
        parse_cookie(value);
      } else {
        m_headers.emplace_back(std::move(name), std::move(value));
      }
      m_buffer.erase(0, line_end + 2);
    }
  }

  inline void HttpResponseParser::parse_body() {
    auto bytes_needed = *m_content_length - m_body_bytes_read;
    auto bytes_available = m_buffer.size();
    if(bytes_available < bytes_needed) {
      if(bytes_available > 0) {
        append(m_body, m_buffer.data(), bytes_available);
        m_body_bytes_read += bytes_available;
        m_buffer.clear();
      }
      return;
    }
    if(bytes_needed > 0) {
      append(m_body, m_buffer.data(), bytes_needed);
      m_buffer.erase(0, bytes_needed);
    }
    finalize_response();
  }

  inline void HttpResponseParser::parse_chunked_size() {
    auto line_end = find_line_end();
    if(line_end == std::string::npos) {
      return;
    }
    auto line = std::string_view(m_buffer.data(), line_end);
    try {
      m_chunk_size = std::stoull(std::string(line), nullptr, 16);
    } catch(...) {
      m_state = ParserState::ERR;
      return;
    }
    m_buffer.erase(0, line_end + 2);
    if(m_chunk_size == 0) {
      m_state = ParserState::CHUNKED_END;
    } else {
      m_state = ParserState::CHUNKED_DATA;
      m_chunk_bytes_read = 0;
    }
  }

  inline void HttpResponseParser::parse_chunked_data() {
    auto bytes_needed = m_chunk_size - m_chunk_bytes_read + 2;
    auto bytes_available = m_buffer.size();
    if(bytes_available < bytes_needed) {
      auto data_available = std::min(
        bytes_available, m_chunk_size - m_chunk_bytes_read);
      if(data_available > 0) {
        append(m_body, m_buffer.data(), data_available);
        m_chunk_bytes_read += data_available;
        m_buffer.erase(0, data_available);
      }
      return;
    }
    auto data_to_read = m_chunk_size - m_chunk_bytes_read;
    if(data_to_read > 0) {
      append(m_body, m_buffer.data(), data_to_read);
      m_buffer.erase(0, data_to_read);
    }
    if(m_buffer.size() < 2 || m_buffer[0] != '\r' || m_buffer[1] != '\n') {
      m_state = ParserState::ERR;
      return;
    }
    m_buffer.erase(0, 2);
    m_state = ParserState::CHUNKED_SIZE;
  }

  inline void HttpResponseParser::parse_chunked_end() {
    if(m_buffer.size() < 2) {
      return;
    }
    if(m_buffer[0] != '\r' || m_buffer[1] != '\n') {
      m_state = ParserState::ERR;
      return;
    }
    m_buffer.erase(0, 2);
    finalize_response();
  }

  inline void HttpResponseParser::parse_cookie(std::string_view source) {
    auto separator = source.find(';');
    if(separator == std::string_view::npos) {
      separator = source.size();
    }
    auto cookie_value = source.substr(0, separator);
    auto equals_position = cookie_value.find('=');
    if(equals_position == std::string_view::npos) {
      m_cookies.emplace_back(std::string(), std::string(cookie_value));
      return;
    }
    auto name = std::string(cookie_value.substr(0, equals_position));
    auto value = std::string(cookie_value.substr(equals_position + 1));
    auto cookie = Cookie(std::move(name), std::move(value));
    auto pos = separator;
    while(pos < source.size()) {
      if(source[pos] == ';') {
        pos += 2;
      }
      if(pos >= source.size()) {
        break;
      }
      auto next_separator = source.find(';', pos);
      if(next_separator == std::string_view::npos) {
        next_separator = source.size();
      }
      auto attribute = source.substr(pos, next_separator - pos);
      auto attr_equals = attribute.find('=');
      if(attr_equals == std::string_view::npos) {
        auto attr_name = std::string(attribute);
        if(boost::iequals(attr_name, "HttpOnly")) {
          cookie.set_http_only(true);
        } else if(boost::iequals(attr_name, "Secure")) {
          cookie.set_secure(true);
        }
      } else {
        auto attr_name = std::string(attribute.substr(0, attr_equals));
        auto attr_value = std::string(attribute.substr(attr_equals + 1));
        if(boost::iequals(attr_name, "path")) {
          cookie.set_path(attr_value);
        } else if(boost::iequals(attr_name, "domain")) {
          cookie.set_domain(attr_value);
        }
      }
      pos = next_separator;
    }
    m_cookies.push_back(std::move(cookie));
  }

  inline void HttpResponseParser::finalize_response() {
    m_responses.emplace_back(m_version, m_status_code, std::move(m_headers),
      std::move(m_cookies), std::move(m_body));
    reset_parser();
  }

  inline void HttpResponseParser::reset_parser() {
    m_state = ParserState::STATUS_LINE;
    m_headers.clear();
    m_cookies.clear();
    m_content_length.reset();
    m_transfer_encoding = TransferEncoding::NONE;
    m_chunk_size = 0;
    m_chunk_bytes_read = 0;
    m_body_bytes_read = 0;
    m_body = SharedBuffer();
  }

  inline std::size_t HttpResponseParser::find_line_end() const {
    if(m_buffer.size() < 2) {
      return std::string::npos;
    }
    for(auto i = std::size_t(0); i < m_buffer.size() - 1; ++i) {
      if(m_buffer[i] == '\r' && m_buffer[i + 1] == '\n') {
        return i;
      }
    }
    return std::string::npos;
  }
}

#endif
