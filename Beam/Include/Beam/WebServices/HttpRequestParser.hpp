#ifndef BEAM_HTTPREQUESTPARSER_HPP
#define BEAM_HTTPREQUESTPARSER_HPP
#include <deque>
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/InvalidHttpRequestException.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpRequestParser
      \brief Parses an HTTP request.
   */
  class HttpRequestParser : private boost::noncopyable {
    public:

      //! Constructs an HttpRequestParser.
      HttpRequestParser();

      //! Feeds the parser additional characters to parse.
      /*!
        \param c The first character to feed.
        \param size The number of characters to feed.
      */
      void Feed(const char* c, std::size_t size);

      //! Returns the next HttpRequest.
      boost::optional<HttpRequest> GetNextRequest();

    private:
      enum class ParserState {
        METHOD,
        HEADER,
        BODY,
        ERR
      };
      ParserState m_parserState;
      HttpMethod m_method;
      boost::optional<Uri> m_uri;
      HttpVersion m_version;
      std::vector<HttpHeader> m_headers;
      SpecialHeaders m_specialHeaders;
      std::vector<Cookie> m_cookies;
      IO::SharedBuffer m_body;
      std::deque<HttpRequest> m_requests;
      IO::SharedBuffer m_buffer;

      void ParseMethod(const char* c, std::size_t size);
      void ParseHeader(const char* c, std::size_t size);
      void ParseCookie(const char* source, int length);
      void ParseCookies(const std::string& source);
      void ParseBody(const char* c);
  };

  inline HttpRequestParser::HttpRequestParser()
      : m_parserState{ParserState::METHOD} {}

  inline void HttpRequestParser::Feed(const char* c, std::size_t size) {
    const auto LINE_LENGTH = 2;
    if(m_parserState == ParserState::METHOD) {
      auto end = static_cast<const char*>(std::memchr(c, '\r', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      }
      if(m_buffer.IsEmpty()) {
        ParseMethod(c, (end - c));
      } else {
        m_buffer.Append(c, end - c);
        ParseMethod(m_buffer.GetData(), m_buffer.GetSize());
        m_buffer.Reset();
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      size -= (end - c) + 1;
      c = end + 1;
      m_parserState = ParserState::HEADER;
    }
    while(m_parserState == ParserState::HEADER) {
      if(size == 0) {
        return;
      }
      auto end = static_cast<const char*>(std::memchr(c, '\r', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      } else if(end == c + 1) {
        ++c;
        --size;
        m_parserState = ParserState::BODY;
        break;
      }
      if(m_buffer.IsEmpty()) {
        ParseHeader(c, (end - c));
      } else {
        m_buffer.Append(c, end - c);
        ParseHeader(m_buffer.GetData(), m_buffer.GetSize());
        m_buffer.Reset();
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      size -= (end - c) + 1;
      c = end + 1;
      m_parserState = ParserState::HEADER;
    }
    if(m_parserState == ParserState::BODY) {
      if(size == 0) {
        return;
      }
      if(m_buffer.GetSize() + size <
          m_specialHeaders.m_contentLength + LINE_LENGTH) {
        m_buffer.Append(c, size);
        return;
      }
      if(m_buffer.IsEmpty()) {
        ParseBody(c);
        size -= m_specialHeaders.m_contentLength + LINE_LENGTH;
        c += m_specialHeaders.m_contentLength + LINE_LENGTH;
      } else {
        auto length = (m_specialHeaders.m_contentLength + LINE_LENGTH) -
          m_buffer.GetSize();
        m_buffer.Append(c, length);
        ParseBody(m_buffer.GetData());
        m_buffer.Reset();
        size -= length;
        c += length;
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      m_requests.emplace_back(m_version, m_method, std::move(*m_uri),
        std::move(m_headers), m_specialHeaders, std::move(m_cookies),
        std::move(m_body));
      m_uri.reset();
      m_headers.clear();
      m_cookies.clear();
      m_body.Reset();
      m_parserState = ParserState::METHOD;
      if(size != 0) {
        m_buffer.Append(c, size);
        auto tempBuffer = std::move(m_buffer);
        m_buffer = IO::SharedBuffer{};
        Feed(tempBuffer.GetData(), tempBuffer.GetSize());
      }
    }
  }

  inline boost::optional<HttpRequest> HttpRequestParser::GetNextRequest() {
    if(m_requests.empty()) {
      if(m_parserState == ParserState::ERR) {
        BOOST_THROW_EXCEPTION(InvalidHttpRequestException{});
      }
      return boost::none;
    }
    auto request = std::move(m_requests.front());
    m_requests.pop_front();
    return std::move(request);
  }

  inline void HttpRequestParser::ParseMethod(const char* c, std::size_t size) {
    static const auto HTTP_VERSION_SIZE = 8;
    if(size >= 4 && std::memcmp(c, "GET ", 4) == 0) {
      m_method = HttpMethod::GET;
      c += 4;
      size -= 4;
    } else if(size >= 5 && std::memcmp(c, "POST ", 5) == 0) {
      m_method = HttpMethod::POST;
      c += 5;
      size -= 5;
    } else {
      m_parserState = ParserState::ERR;
      return;
    }
    auto uriEnd = static_cast<const char*>(std::memchr(c, ' ', size));
    if(uriEnd == nullptr) {
      m_parserState = ParserState::ERR;
      return;
    }
    try {
      m_uri.emplace(c, uriEnd);
    } catch(const MalformedUriException&) {
      m_parserState = ParserState::ERR;
      return;
    }
    size -= (uriEnd - c) + 1;
    c = uriEnd + 1;
    if(size != HTTP_VERSION_SIZE) {
      m_parserState = ParserState::ERR;
      return;
    }
    if(std::memcmp(c, "HTTP/1.0", HTTP_VERSION_SIZE) == 0) {
      m_version = HttpVersion::Version1_0();
    } else if(std::memcmp(c, "HTTP/1.1", HTTP_VERSION_SIZE) == 0) {
      m_version = HttpVersion::Version1_1();
    } else {
      m_parserState = ParserState::ERR;
    }
    m_specialHeaders = SpecialHeaders{m_version};
  }

  inline void HttpRequestParser::ParseHeader(const char* c, std::size_t size) {
    if(*c != '\n') {
      m_parserState = ParserState::ERR;
      return;
    }
    ++c;
    --size;
    auto nameEnd = static_cast<const char*>(std::memchr(c, ':', size));
    if(nameEnd == nullptr) {
      m_parserState = ParserState::ERR;
      return;
    }
    auto nameLength = static_cast<unsigned int>(nameEnd - c);
    std::string name{c, nameLength};
    c += nameLength + 1;
    size -= nameLength + 1;
    if(*c != ' ') {
      m_parserState = ParserState::ERR;
      return;
    }
    ++c;
    --size;
    std::string value{c, size};
    if(m_specialHeaders.m_contentLength == 0 &&
        boost::iequals(name, "Content-Length")) {
      m_specialHeaders.m_contentLength = std::stoul(value);
    } else if(boost::iequals(name, "Connection")) {
      m_specialHeaders.m_connection = ConnectionHeader::CLOSE;
      if(boost::ifind_first(value, "Upgrade")) {
        m_specialHeaders.m_connection = ConnectionHeader::UPGRADE;
      } else if(boost::ifind_first(value, "keep-alive")) {
        m_specialHeaders.m_connection = ConnectionHeader::KEEP_ALIVE;
      }
    } else {
      if(m_cookies.empty() && boost::iequals(name, "Cookie")) {
        ParseCookies(value);
      } else {
        m_headers.emplace_back(std::move(name), std::move(value));
      }
    }
  }

  inline void HttpRequestParser::ParseCookie(const char* source, int length) {
    auto separator = static_cast<const char*>(std::memchr(source, '=', length));
    if(separator == nullptr) {
      m_cookies.emplace_back(std::string{},
        std::string{source, static_cast<unsigned int>(length)});
    } else {
      m_cookies.emplace_back(std::string{source, separator},
        std::string{separator + 1, static_cast<std::string::size_type>(length) -
        (separator - source) - 1});
    }
  }

  inline void HttpRequestParser::ParseCookies(const std::string& source) {
    std::string::size_type front = 0;
    while(front < source.size()) {
      auto separator = source.find(';', front);
      if(separator == std::string::npos) {
        separator = source.size();
      }
      ParseCookie(source.c_str() + front, separator - front);
      front = separator + 2;
    }
  }

  inline void HttpRequestParser::ParseBody(const char* c) {
    auto size = m_specialHeaders.m_contentLength + 2;
    if(*c != '\r' || *(c + 1) != '\n') {
      m_parserState = ParserState::ERR;
      return;
    }
    if(m_specialHeaders.m_contentLength != 0) {
      m_body.Append(c + 2, m_specialHeaders.m_contentLength);
    }
  }
}
}

#endif
