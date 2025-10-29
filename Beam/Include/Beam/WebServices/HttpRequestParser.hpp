#ifndef BEAM_HTTP_REQUEST_PARSER_HPP
#define BEAM_HTTP_REQUEST_PARSER_HPP
#include <cstring>
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/InvalidHttpRequestException.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {

  /** Parses an HTTP request. */
  class HttpRequestParser {
    public:

      /** Constructs an HttpRequestParser. */
      HttpRequestParser() noexcept;

      /**
       * Feeds the parser additional characters to parse.
       * @param source The characters to feed to the parser.
       */
      void feed(std::string_view source);

      /** Returns the next HttpRequest. */
      boost::optional<HttpRequest> get_next_request();

    private:
      enum class ParserState {
        REQUEST_LINE,
        HEADERS,
        BODY,
        ERR
      };
      ParserState m_state;
      std::string m_buffer;
      HttpMethod m_method;
      boost::optional<Uri> m_uri;
      HttpVersion m_version;
      std::vector<HttpHeader> m_headers;
      SpecialHeaders m_special_headers;
      std::vector<Cookie> m_cookies;
      SharedBuffer m_body;
      std::size_t m_body_bytes_read;
      std::deque<HttpRequest> m_requests;

      HttpRequestParser(const HttpRequestParser&) = delete;
      HttpRequestParser(HttpRequestParser&&) = delete;
      void parse();
      void parse_request_line();
      void parse_headers();
      void parse_body();
      void parse_cookie(std::string_view source);
      void finalize_request();
      void reset_parser();
      std::size_t find_line_end() const;
  };

  inline HttpRequestParser::HttpRequestParser() noexcept
    : m_state(ParserState::REQUEST_LINE),
      m_body_bytes_read(0) {}

  inline void HttpRequestParser::feed(std::string_view source) {
    m_buffer.append(source);
    parse();
  }

  inline boost::optional<HttpRequest> HttpRequestParser::get_next_request() {
    if(!m_requests.empty()) {
      auto request = std::move(m_requests.front());
      m_requests.pop_front();
      return request;
    }
    if(m_state == ParserState::ERR) {
      boost::throw_with_location(InvalidHttpRequestException());
    }
    return boost::none;
  }

  inline void HttpRequestParser::parse() {
    while(true) {
      if(m_state == ParserState::REQUEST_LINE) {
        parse_request_line();
        if(m_state != ParserState::HEADERS) {
          return;
        }
      }
      if(m_state == ParserState::HEADERS) {
        parse_headers();
        if(m_state != ParserState::BODY) {
          return;
        }
      }
      if(m_state == ParserState::BODY) {
        parse_body();
        if(m_state != ParserState::REQUEST_LINE) {
          return;
        }
      }
      if(m_state == ParserState::ERR) {
        return;
      }
    }
  }

  inline void HttpRequestParser::parse_request_line() {
    auto line_end = find_line_end();
    if(line_end == std::string::npos) {
      return;
    }
    auto line = std::string_view(m_buffer.data(), line_end);
    auto method_end = line.find(' ');
    if(method_end == std::string_view::npos) {
      m_state = ParserState::ERR;
      return;
    }
    auto method = HttpMethod::from(line.substr(0, method_end));
    if(method == HttpMethod::NONE) {
      m_state = ParserState::ERR;
      return;
    }
    m_method = method;
    auto uri_start = method_end + 1;
    auto uri_end = line.find(' ', uri_start);
    if(uri_end == std::string_view::npos) {
      m_state = ParserState::ERR;
      return;
    }
    auto uri_text = line.substr(uri_start, uri_end - uri_start);
    try {
      m_uri.emplace(uri_text);
    } catch(const MalformedUriException&) {
      m_state = ParserState::ERR;
      return;
    }
    auto version_text = line.substr(uri_end + 1);
    if(version_text == "HTTP/1.0") {
      m_version = HttpVersion::version_1_0();
    } else if(version_text == "HTTP/1.1") {
      m_version = HttpVersion::version_1_1();
    } else {
      m_state = ParserState::ERR;
      return;
    }
    m_special_headers = SpecialHeaders(m_version);
    m_buffer.erase(0, line_end + 2);
    m_state = ParserState::HEADERS;
  }

  inline void HttpRequestParser::parse_headers() {
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
        if(m_buffer.size() >= 2 && m_buffer[0] == '\r' && m_buffer[1] == '\n') {
          m_buffer.erase(0, 2);
        }
        m_state = ParserState::BODY;
        m_body_bytes_read = 0;
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
        try {
          m_special_headers.m_content_length = std::stoull(value);
        } catch(...) {
          m_state = ParserState::ERR;
          return;
        }
      } else if(boost::iequals(name, "Connection")) {
        if(boost::ifind_first(value, "Upgrade")) {
          m_special_headers.m_connection = ConnectionHeader::UPGRADE;
        } else if(boost::ifind_first(value, "keep-alive")) {
          m_special_headers.m_connection = ConnectionHeader::KEEP_ALIVE;
        } else {
          m_special_headers.m_connection = ConnectionHeader::CLOSE;
        }
      } else if(boost::iequals(name, "Host")) {
        m_special_headers.m_host = value;
      } else if(boost::iequals(name, "Cookie")) {
        auto cookie_start = std::size_t(0);
        while(cookie_start < value.size()) {
          auto cookie_end = value.find(';', cookie_start);
          if(cookie_end == std::string::npos) {
            cookie_end = value.size();
          }
          auto cookie_str = std::string_view(value).substr(
            cookie_start, cookie_end - cookie_start);
          parse_cookie(cookie_str);
          cookie_start = cookie_end + 2;
        }
      } else {
        m_headers.emplace_back(std::move(name), std::move(value));
      }
      m_buffer.erase(0, line_end + 2);
    }
  }

  inline void HttpRequestParser::parse_body() {
    auto bytes_needed = m_special_headers.m_content_length - m_body_bytes_read;
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
    finalize_request();
  }

  inline void HttpRequestParser::parse_cookie(std::string_view source) {
    auto equals_position = source.find('=');
    if(equals_position == std::string_view::npos) {
      m_cookies.emplace_back(std::string(), std::string(source));
    } else {
      auto name = std::string(source.substr(0, equals_position));
      auto value = std::string(source.substr(equals_position + 1));
      m_cookies.emplace_back(std::move(name), std::move(value));
    }
  }

  inline void HttpRequestParser::finalize_request() {
    m_requests.emplace_back(m_version, m_method, std::move(*m_uri),
      std::move(m_headers), m_special_headers, std::move(m_cookies),
      std::move(m_body));
    reset_parser();
  }

  inline void HttpRequestParser::reset_parser() {
    m_state = ParserState::REQUEST_LINE;
    m_uri.reset();
    m_headers.clear();
    m_cookies.clear();
    m_body = SharedBuffer();
    m_body_bytes_read = 0;
  }

  inline std::size_t HttpRequestParser::find_line_end() const {
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
