#ifndef BEAM_HTTPREQUESTPARSER_HPP
#define BEAM_HTTPREQUESTPARSER_HPP
#include <deque>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/WebServices/HttpServerRequest.hpp"
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
        URI,
        VERSION,
        BODY,
        ERROR
      };
      ParserState m_parserState;
      HttpMethod m_method;
      boost::optional<Uri> m_uri;
      std::deque<HttpServerRequest> m_requests;
      IO::SharedBuffer m_buffer;
      const char* m_cursor;
  };

  inline HttpRequestParser::HttpRequestParser()
      : m_parserState{ParserState::METHOD},
        m_cursor{m_buffer.GetData()} {}

  inline void HttpRequestParser::Feed(const char* c, std::size_t size) {
    if(m_parserState == ParserState::METHOD) {
      if(m_buffer.IsEmpty()) {
        if(size >= 4 && std::memcmp(c, "GET ", 4) == 0) {
          m_method = HttpMethod::GET;
          c += 4;
          m_parserState = ParserState::URI;
        } else if(size >= 5 && std::memcmp(c, "POST ", 5) == 0) {
          m_method = HttpMethod::POST;
          c += 5;
          m_parserState = ParserState::URI;
        } else if(std::strchr(c, ' ') != nullptr) {
          m_parserState = ParserState::ERROR;
        } else {
          m_buffer.Append(c, size);
          return;
        }
      } else {

        // TODO
      }
    }
    if(m_parserState == ParserState::URI) {
      if(m_buffer.IsEmpty()) {
        auto end = std::strchr(c, ' ');
        if(end == nullptr) {
          m_buffer.Append(c, size);
          return;
        }
        try {
          m_uri.emplace(c, end);
        } catch(const MalformedUriException&) {
          m_parserState = ParserState::ERROR;
          return;
        }
        c = end + 1;
        m_parserState = ParserState::VERSION;
      } else {

        // TODO
      }
    }
  }

  inline boost::optional<HttpServerRequest> HttpRequestParser::
      GetNextRequest() {
    if(m_requests.empty()) {
      if(m_parserState == ParserState::ERROR) {
        BOOST_THROW_EXCEPTION(InvalidHttpRequestException{});
      }
      return boost::none;
    }
    auto request = std::move(m_requests.front());
    m_requests.pop_front();
    return std::move(request);
  }
}
}

#endif
