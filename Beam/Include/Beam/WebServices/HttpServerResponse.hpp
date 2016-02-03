#ifndef BEAM_HTTPSERVERRESPONSE_HPP
#define BEAM_HTTPSERVERRESPONSE_HPP
#include <vector>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpStatusCode.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpServerResponse
      \brief Handles the response to an HttpServerRequest.
   */
  class HttpServerResponse {
    public:

      //! Constructs an HttpServerResponse with a status of OK.
      HttpServerResponse();

      //! Constructs an HttpServerResponse.
      /*!
        \param statusCode The HttpStatusCode.
      */
      HttpServerResponse(HttpStatusCode statusCode);

      //! Sets the status code.
      void SetStatusCode(HttpStatusCode statusCode);

      //! Sets a header, adding it if it doesn't exist.
      /*!
        \param header The header to set.
      */
      void SetHeader(HttpHeader header);

      //! Sets a Cookie, adding it if it doesn't exist.
      /*!
        \param cookie The Cookie to set.
      */
      void SetCookie(Cookie cookie);

      //! Sets the body.
      void SetBody(IO::SharedBuffer body);

    private:
      HttpStatusCode m_statusCode;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      IO::SharedBuffer m_body;
  };

  inline HttpServerResponse::HttpServerResponse()
      : HttpServerResponse{HttpStatusCode::OK} {}

  inline HttpServerResponse::HttpServerResponse(HttpStatusCode statusCode)
      : m_statusCode{statusCode} {}

  inline void HttpServerResponse::SetStatusCode(HttpStatusCode statusCode) {
    m_statusCode = statusCode;
  }

  inline void HttpServerResponse::SetHeader(HttpHeader header) {
    auto h = std::find_if(m_headers.begin(), m_headers.end(),
      [&] (const HttpHeader& value) {
        return value.GetName() == header.GetName();
      });
    if(h == m_headers.end()) {
      m_headers.push_back(std::move(header));
    } else {
      *h = std::move(header);
    }
  }

  inline void HttpServerResponse::SetCookie(Cookie cookie) {
    auto c = std::find_if(m_cookies.begin(), m_cookies.end(),
      [&] (const Cookie& value) {
        return value.GetName() == cookie.GetName();
      });
    if(c == m_cookies.end()) {
      m_cookies.push_back(std::move(cookie));
    } else {
      *c = std::move(cookie);
    }
  }

  inline void HttpServerResponse::SetBody(IO::SharedBuffer body) {
    m_body = std::move(body);
  }
}
}

#endif
