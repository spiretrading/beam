#ifndef BEAM_HTTPRESPONSEPARSER_HPP
#define BEAM_HTTPRESPONSEPARSER_HPP
#include <cstdlib>
#include <deque>
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/InvalidHttpResponseException.hpp"
#include "Beam/WebServices/TransferEncoding.hpp"
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

      //! Returns the remaining unparsed Buffer.
      IO::SharedBuffer GetRemainingBuffer() const;

    private:
      enum class ParserState {
        VERSION,
        HEADER,
        BODY,
        ERR
      };
      enum class ChunkState {
        SKIP_LINE,
        SIZE,
        BODY,
        ERR
      };
      ParserState m_parserState;
      HttpVersion m_version;
      HttpStatusCode m_statusCode;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      boost::optional<std::size_t> m_contentLength;
      TransferEncoding m_transferEncoding;
      std::size_t m_chunkSize;
      ChunkState m_chunkState;
      IO::SharedBuffer m_body;
      std::deque<HttpResponse> m_responses;
      IO::SharedBuffer m_buffer;

      void PushResponse();
      void ParseVersion(const char* c, std::size_t size);
      void ParseHeader(const char* c, std::size_t size);
      std::tuple<std::string, std::string> ParseCookiePair(const char* source,
        int length);
      void ParseCookie(const std::string& source);
      void ParseBodyWithContentLength(const char* c, std::size_t size);
      void ParseBodyWithContentLength(const char* c);
      void ParseChunkedBody(const char* c, std::size_t size);
  };

  inline HttpResponseParser::HttpResponseParser()
      : m_parserState{ParserState::VERSION},
        m_transferEncoding{TransferEncoding::NONE},
        m_chunkSize{0},
        m_chunkState{ChunkState::SKIP_LINE} {}

  inline void HttpResponseParser::Feed(const char* c, std::size_t size) {
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
      if(m_contentLength.is_initialized()) {
        ParseBodyWithContentLength(c, size);
      } else if(m_transferEncoding == TransferEncoding::CHUNKED) {
        if(m_buffer.IsEmpty()) {
          ParseChunkedBody(c, size);
        } else {
          m_buffer.Append(c, size);
          auto buffer = std::move(m_buffer);
          m_buffer.Reset();
          ParseChunkedBody(buffer.GetData(), buffer.GetSize());
        }
        if(m_chunkState == ChunkState::ERR) {
          m_parserState = ParserState::ERR;
          return;
        }
      } else {
        PushResponse();
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

  inline IO::SharedBuffer HttpResponseParser::GetRemainingBuffer() const {
    return m_buffer;
  }

  inline void HttpResponseParser::PushResponse() {
    m_responses.emplace_back(m_version, m_statusCode, std::move(m_headers),
      std::move(m_cookies), std::move(m_body));
    m_headers.clear();
    m_cookies.clear();
    m_contentLength.reset();
    m_transferEncoding = TransferEncoding::NONE;
    m_chunkSize = 0;
    m_chunkState = ChunkState::SKIP_LINE;
    m_body.Reset();
    m_parserState = ParserState::VERSION;
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
    if(statusCodeEnd == nullptr) {
      statusCodeEnd = c + size;
    }
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
    if(name == "Content-Length") {
      if(!m_contentLength.is_initialized()) {
        try {
          m_contentLength = std::stoul(value);
        } catch(const std::exception&) {
          m_parserState = ParserState::ERR;
          return;
        }
      }
    } else if(name == "Set-Cookie") {
      ParseCookie(value);
    } else if(name == "Transfer-Encoding") {
      if(value == "chunked") {
        m_transferEncoding = TransferEncoding::CHUNKED;
      }
      m_headers.emplace_back(std::move(name), std::move(value));
    } else {
      m_headers.emplace_back(std::move(name), std::move(value));
    }
  }

  inline std::tuple<std::string, std::string>
      HttpResponseParser::ParseCookiePair(const char* source, int length) {
    auto separator = static_cast<const char*>(std::memchr(source, '=', length));
    if(separator == nullptr) {
      return std::make_tuple(std::string{},
        std::string{source, static_cast<unsigned int>(length)});
    } else {
      return std::make_tuple(std::string{source, separator},
        std::string{separator + 1, static_cast<std::string::size_type>(length) -
        (separator - source) - 1});
    }
  }

  inline void HttpResponseParser::ParseCookie(const std::string& source) {
    std::string::size_type front = 0;
    auto separator = source.find(';', front);
    if(separator == std::string::npos) {
      separator = source.size();
    }
    auto cookiePair = ParseCookiePair(source.c_str() + front,
      separator - front);
    Cookie cookie{std::get<0>(cookiePair), std::get<1>(cookiePair)};
    front = separator + 2;
    while(front < source.size()) {
      separator = source.find(';', front);
      if(separator == std::string::npos) {
        separator = source.size();
      }
      auto token = source.c_str() + front;
      auto length = separator - front;
      front = separator + 2;
      auto delimiter = std::memchr(token, '=', length);
      if(delimiter == nullptr) {
        std::string directive{token, static_cast<unsigned int>(length)}; 
        if(boost::iequals(directive, "HttpOnly")) {
          cookie.SetHttpOnly(true);
        } else if(boost::iequals(directive, "Secure")) {
          cookie.SetSecure(true);
        }
      } else {
        auto directive = ParseCookiePair(token, length);
        if(boost::iequals(std::get<0>(directive), "path")) {
          cookie.SetPath(std::get<1>(directive));
        } else if(boost::iequals(std::get<0>(directive), "domain")) {
          cookie.SetDomain(std::get<1>(directive));
        }
      }
    }
    m_cookies.push_back(std::move(cookie));
  }

  inline void HttpResponseParser::ParseBodyWithContentLength(const char* c,
      std::size_t size) {
    const auto LINE_LENGTH = 2;
    if(size == 0) {
      return;
    }
    if(m_buffer.GetSize() + size < *m_contentLength + LINE_LENGTH) {
      m_buffer.Append(c, size);
      return;
    }
    if(m_buffer.IsEmpty()) {
      ParseBodyWithContentLength(c);
      size -= *m_contentLength + LINE_LENGTH;
      c += *m_contentLength + LINE_LENGTH;
    } else {
      auto length = (*m_contentLength + LINE_LENGTH) - m_buffer.GetSize();
      m_buffer.Append(c, length);
      ParseBodyWithContentLength(m_buffer.GetData());
      m_buffer.Reset();
      size -= length;
      c += length;
    }
    if(m_parserState == ParserState::ERR) {
      return;
    }
    PushResponse();
    if(size != 0) {
      m_buffer.Append(c, size);
      auto tempBuffer = std::move(m_buffer);
      m_buffer = IO::SharedBuffer{};
      Feed(tempBuffer.GetData(), tempBuffer.GetSize());
    }
  }

  inline void HttpResponseParser::ParseBodyWithContentLength(const char* c) {
    auto size = *m_contentLength + 2;
    if(*c != '\r' || *(c + 1) != '\n') {
      m_parserState = ParserState::ERR;
      return;
    }
    if(*m_contentLength != 0) {
      m_body.Append(c + 2, *m_contentLength);
    }
  }

  inline void HttpResponseParser::ParseChunkedBody(const char* c,
      std::size_t size) {
    const auto LINE_LENGTH = 2;
    const auto HEX_BASE = 16;
    if(m_chunkState == ChunkState::SKIP_LINE) {
      if(size < LINE_LENGTH) {
        m_buffer.Append(c, size);
        return;
      }
      if(*c == '\r' && *(c + 1) == '\n') {
        c += 2;
        size -= 2;
      } else {
        m_chunkState = ChunkState::ERR;
        return;
      }
      m_chunkState = ChunkState::SIZE;
    }
    if(m_chunkState == ChunkState::SIZE) {
      if(size == 0) {
        return;
      }
      auto end = static_cast<const char*>(std::memchr(c, '\n', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      }
      std::string value(c, static_cast<int>(end - c - 1));
      try {
        m_chunkSize = static_cast<std::size_t>(std::stoull(value, NULL,
          HEX_BASE));
      } catch(const std::exception&) {
        m_chunkState = ChunkState::ERR;
        return;
      }
      size -= (end - c) + 1;
      c = end + 1;
      if(m_chunkSize == 0) {
        PushResponse();
        if(size != 0) {
          m_buffer.Append(c, size);
          auto tempBuffer = std::move(m_buffer);
          m_buffer = IO::SharedBuffer{};
          Feed(tempBuffer.GetData(), tempBuffer.GetSize());
        }
      } else {
        m_chunkState = ChunkState::BODY;
      }
    }
    if(m_chunkState == ChunkState::BODY) {
      if(size < m_chunkSize + 2) {
        m_buffer.Append(c, size);
        return;
      } else {
        m_body.Append(c, m_chunkSize);
        if(c[m_chunkSize] == '\r' && c[m_chunkSize + 1] == '\n') {
          c += m_chunkSize + 2;
          size -= m_chunkSize + 2;
          m_chunkState = ChunkState::SIZE;
          ParseChunkedBody(c, size);
        } else {
          m_chunkState = ChunkState::ERR;
        }
      }
    }
  }
}
}

#endif
