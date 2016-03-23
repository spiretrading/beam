#ifndef BEAM_HTTPRESPONSEPARSER_HPP
#define BEAM_HTTPRESPONSEPARSER_HPP
#include <deque>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/InvalidHttpResponseException.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpResponseParser
      \brief Parses an HTTP response.
   */
  class HttpResponseParser : private boost::noncopyable {
    public:

      //! Constructs an HttpResponseParser.
      HttpResponseParser();

      //! Feeds the parser additional characters to parse.
      /*!
        \param c The first character to feed.
        \param size The number of characters to feed.
      */
      void Feed(const char* c, std::size_t size);

      //! Returns the next HttpResponse.
      boost::optional<HttpResponse> GetNextResponse();

    private:
      enum class ParserState {
        VERSION,
        HEADER,
        BODY,
        ERR
      };
      ParserState m_parserState;
      HttpVersion m_version;
      HttpStatusCode m_statusCode;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      std::size_t m_contentLength;
      IO::SharedBuffer m_body;
      std::deque<HttpResponse> m_responses;
      IO::SharedBuffer m_buffer;

      void ParseVersion(const char* c, std::size_t size);
      void ParseHeader(const char* c, std::size_t size);
      void ParseCookie(const char* source, int length);
      void ParseCookies(const std::string& source);
      void ParseBody(const char* c);
  };

  inline HttpResponseParser::HttpResponseParser()
      : m_parserState{ParserState::VERSION},
        m_contentLength{0} {}

  inline void HttpResponseParser::Feed(const char* c, std::size_t size) {
    const auto LINE_LENGTH = 2;
    if(m_parserState == ParserState::VERSION) {
      auto end = static_cast<const char*>(std::memchr(c, '\r', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      }
      if(m_buffer.IsEmpty()) {
        ParseVersion(c, (end - c));
      } else {
        m_buffer.Append(c, end - c);
        ParseVersion(m_buffer.GetData(), m_buffer.GetSize());
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
      if(m_buffer.GetSize() + size < m_contentLength + LINE_LENGTH) {
        m_buffer.Append(c, size);
        return;
      }
      if(m_buffer.IsEmpty()) {
        ParseBody(c);
        size -= m_contentLength + LINE_LENGTH;
        c += m_contentLength + LINE_LENGTH;
      } else {
        auto length = (m_contentLength + LINE_LENGTH) - m_buffer.GetSize();
        m_buffer.Append(c, length);
        ParseBody(c);
        m_buffer.Reset();
        size -= length;
        c += length;
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      m_responses.emplace_back(m_version, m_statusCode, std::move(m_headers),
        std::move(m_cookies), std::move(m_body));
      m_headers.clear();
      m_cookies.clear();
      m_contentLength = 0;
      m_body.Reset();
      m_parserState = ParserState::VERSION;
      if(size != 0) {
        m_buffer.Append(c, size);
        auto tempBuffer = std::move(m_buffer);
        m_buffer = IO::SharedBuffer{};
        Feed(tempBuffer.GetData(), tempBuffer.GetSize());
      }
    }
  }

  inline boost::optional<HttpResponse> HttpResponseParser::GetNextResponse() {
    if(m_responses.empty()) {
      if(m_parserState == ParserState::ERR) {
        BOOST_THROW_EXCEPTION(InvalidHttpResponseException{});
      }
      return boost::none;
    }
    auto response = std::move(m_responses.front());
    m_responses.pop_front();
    return std::move(response);
  }

  inline void HttpResponseParser::ParseVersion(const char* c,
      std::size_t size) {
    static const auto HTTP_VERSION_SIZE = 9;
    if(size < HTTP_VERSION_SIZE) {
      m_parserState = ParserState::ERR;
      return;
    }
    if(std::memcmp(c, "HTTP/1.0 ", HTTP_VERSION_SIZE) == 0) {
      m_version = HttpVersion::Version1_0();
    } else if(std::memcmp(c, "HTTP/1.1 ", HTTP_VERSION_SIZE) == 0) {
      m_version = HttpVersion::Version1_1();
    } else {
      m_parserState = ParserState::ERR;
    }
    c += HTTP_VERSION_SIZE;
    size -= HTTP_VERSION_SIZE;
    auto statusCodeEnd = static_cast<const char*>(std::memchr(c, ' ', size));
    int statusCode = 0;
    while(c != statusCodeEnd) {
      statusCode = 10 * statusCode + (*c - '0');
      ++c;
      --size;
    }
    m_statusCode = static_cast<HttpStatusCode>(statusCode);
  }

  inline void HttpResponseParser::ParseHeader(const char* c, std::size_t size) {
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
    if(m_contentLength == 0 && name == "Content-Length") {
      m_contentLength = std::stoul(value);
    } else {
      if(m_cookies.empty() && name == "Cookie") {
        ParseCookies(value);
      } else {
        m_headers.emplace_back(std::move(name), std::move(value));
      }
    }
  }

  inline void HttpResponseParser::ParseCookie(const char* source, int length) {
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

  inline void HttpResponseParser::ParseCookies(const std::string& source) {
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

  inline void HttpResponseParser::ParseBody(const char* c) {
    auto size = m_contentLength + 2;
    if(*c != '\r' || *(c + 1) != '\n') {
      m_parserState = ParserState::ERR;
      return;
    }
    if(m_contentLength != 0) {
      m_body.Append(c + 2, m_contentLength);
    }
  }
}
}

#endif
