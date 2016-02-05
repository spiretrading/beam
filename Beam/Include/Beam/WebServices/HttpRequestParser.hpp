#ifndef BEAM_HTTPREQUESTPARSER_HPP
#define BEAM_HTTPREQUESTPARSER_HPP
#include <deque>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/HttpServerRequest.hpp"
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

      //! Returns the next HttpServerRequest.
      boost::optional<HttpServerRequest> GetNextRequest();

    private:
      enum class ParserState {
        METHOD,
        HEADER,
        ERR
      };
      ParserState m_parserState;
      HttpMethod m_method;
      boost::optional<Uri> m_uri;
      HttpVersion m_version;
      std::deque<HttpServerRequest> m_requests;
      IO::SharedBuffer m_buffer;

      void ParseMethod(const char* c, std::size_t size);
      void ParseHeader(const char* c, std::size_t size);
  };

  inline HttpRequestParser::HttpRequestParser()
      : m_parserState{ParserState::METHOD} {}

  inline void HttpRequestParser::Feed(const char* c, std::size_t size) {
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
  }

  inline boost::optional<HttpServerRequest> HttpRequestParser::
      GetNextRequest() {
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
  }

  inline void HttpRequestParser::ParseHeader(const char* c, std::size_t size) {
  }
}
}

#endif
