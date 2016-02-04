#ifndef BEAM_HTTPSERVERREQUEST_HPP
#define BEAM_HTTPSERVERREQUEST_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpMethod.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpServerRequest
      \brief Represents an HTTP request received by the HttpServer.
   */
  class HttpServerRequest {
    public:

      //! Constructs an HttpServerRequest.
      /*!
        \param version The HTTP version in major/minor format.
        \param method The HTTP method.
        \param uri The URI to perform the <i>method</i> on.
        \param headers The request headers.
        \param cookies The Cookies.
        \param body The message body.
      */
      HttpServerRequest(HttpVersion version, HttpMethod method, Uri uri,
        std::vector<HttpHeader> headers, std::vector<Cookie> cookies,
        IO::SharedBuffer body);

      //! Returns the HTTP version.
      const HttpVersion& GetVersion() const;

      //! Returns the HttpMethod to perform.
      HttpMethod GetMethod() const;

      //! Returns the URI.
      const Uri& GetUri() const;

      //! Returns the value of a header.
      /*!
        \param name The name of the header.
        \return The value of the header with the specified <i>name</i>.
      */
      boost::optional<const std::string&> GetHeader(
        const std::string& name) const;

      //! Returns all headers.
      const std::vector<HttpHeader>& GetHeaders() const;

      //! Returns a Cookie with a specified name.
      /*!
        \param name The name of the Cookie.
        \return The Cookie with the specified name or an empty Cookie if the
                Cookie was not found.
      */
      boost::optional<const Cookie&> GetCookie(const std::string& name) const;

      //! Returns all Cookies.
      const std::vector<Cookie>& GetCookies() const;

      //! Returns the message body.
      const IO::SharedBuffer& GetBody() const;

    private:
      HttpVersion m_version;
      HttpMethod m_method;
      Uri m_uri;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      IO::SharedBuffer m_body;
  };

  inline HttpServerRequest::HttpServerRequest(HttpVersion version,
      HttpMethod method, Uri uri, std::vector<HttpHeader> headers,
      std::vector<Cookie> cookies, IO::SharedBuffer body)
      : m_version{std::move(version)},
        m_method{std::move(method)},
        m_uri{std::move(uri)},
        m_headers{std::move(headers)},
        m_cookies{std::move(cookies)},
        m_body{std::move(body)}  {}

  inline const HttpVersion& HttpServerRequest::GetVersion() const {
    return m_version;
  }

  inline HttpMethod HttpServerRequest::GetMethod() const {
    return m_method;
  }

  inline const Uri& HttpServerRequest::GetUri() const {
    return m_uri;
  }

  inline boost::optional<const std::string&> HttpServerRequest::GetHeader(
      const std::string& name) const {
    auto header = std::find_if(m_headers.begin(), m_headers.end(),
      [&] (const HttpHeader& value) {
        return value.GetName() == name;
      });
    if(header == m_headers.end()) {
      return boost::none;
    }
    return header->GetValue();
  }

  inline const std::vector<HttpHeader>& HttpServerRequest::GetHeaders() const {
    return m_headers;
  }

  inline boost::optional<const Cookie&> HttpServerRequest::GetCookie(
      const std::string& name) const {
    auto cookie = std::find_if(m_cookies.begin(), m_cookies.end(),
      [&] (const Cookie& value) {
        return value.GetName() == name;
      });
    if(cookie == m_cookies.end()) {
      return boost::none;
    }
    return *cookie;
  }

  inline const std::vector<Cookie>& HttpServerRequest::GetCookies() const {
    return m_cookies;
  }

  inline const IO::SharedBuffer& HttpServerRequest::GetBody() const {
    return m_body;
  }
}
}

#endif
