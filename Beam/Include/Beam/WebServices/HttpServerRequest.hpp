#ifndef BEAM_HTTPSERVERREQUEST_HPP
#define BEAM_HTTPSERVERREQUEST_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpMethod.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \enum ConnectionHeader
      \brief The set of acceptable Connection header values.
   */
  enum class ConnectionHeader {

    //! Close the connection.
    CLOSE,

    //! Keep the connection open.
    KEEP_ALIVE,

    //! Upgrade the connection.
    UPGRADE
  };

  /*! \struct SpecialHeaders
      \brief Keeps info about specially designated headers.
   */
  struct SpecialHeaders {

    //! The size of the body.
    std::size_t m_contentLength;

    //! Whether to keep the connection open.
    ConnectionHeader m_connection;

    //! Constructs a default SpecialHeaders field.
    SpecialHeaders();

    //! Constructs a default SpecialHeaders field for a specified HTTP version.
    /*!
      \param version The HTTP version to get the default headers for.
    */
    SpecialHeaders(const HttpVersion& version);
  };

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
        \param specialHeaders The set of specially designated headers.
        \param cookies The Cookies.
        \param body The message body.
      */
      HttpServerRequest(HttpVersion version, HttpMethod method, Uri uri,
        std::vector<HttpHeader> headers, const SpecialHeaders& specialHeaders,
        std::vector<Cookie> cookies, IO::SharedBuffer body);

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

      //! Returns the special headers.
      const SpecialHeaders& GetSpecialHeaders() const;

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
      SpecialHeaders m_specialHeaders;
      std::vector<Cookie> m_cookies;
      IO::SharedBuffer m_body;
      mutable Threading::Sync<std::string> m_contentLength;
  };

  inline std::ostream& operator <<(std::ostream& sink,
      ConnectionHeader connectionHeader) {
    if(connectionHeader == ConnectionHeader::CLOSE) {
      sink << "close";
    } else if(connectionHeader == ConnectionHeader::KEEP_ALIVE) {
      sink << "keep-alive";
    } else {
      sink << "Upgrade";
    }
    return sink;
  }

  inline std::ostream& operator <<(std::ostream& sink,
      const HttpServerRequest& request) {
    sink << request.GetMethod() << ' ' << request.GetUri() << ' ' <<
      request.GetVersion() << "\r\n";
    for(auto& header : request.GetHeaders()) {
      sink << header.GetName() << ": " << header.GetValue() << "\r\n";
    }
    if(!request.GetCookies().empty()) {
      sink << "Cookie: ";
      bool isFirst = true;
      for(auto& cookie : request.GetCookies()) {
        if(!isFirst) {
          sink << "; ";
        } else {
          isFirst = false;
        }
        sink << cookie.GetName() << '=' << cookie.GetValue();
      }
    }
    sink << "Content-Length: " << request.GetSpecialHeaders().m_contentLength <<
      "\r\n";
    sink << "Connection: " << request.GetSpecialHeaders().m_connection <<
      "\r\n";
    sink << "\r\n";
    sink << request.GetBody();
    return sink;
  }

  inline SpecialHeaders::SpecialHeaders()
      : m_contentLength{0},
        m_connection{ConnectionHeader::KEEP_ALIVE} {}

  inline SpecialHeaders::SpecialHeaders(const HttpVersion& version) {
    if(version == HttpVersion::Version1_1()) {
      m_contentLength = 0;
      m_connection = ConnectionHeader::KEEP_ALIVE;
    } else {
      m_contentLength = 0;
      m_connection = ConnectionHeader::CLOSE;
    }
  }

  inline HttpServerRequest::HttpServerRequest(HttpVersion version,
      HttpMethod method, Uri uri, std::vector<HttpHeader> headers,
      const SpecialHeaders& specialHeaders, std::vector<Cookie> cookies,
      IO::SharedBuffer body)
      : m_version{std::move(version)},
        m_method{std::move(method)},
        m_uri{std::move(uri)},
        m_headers{std::move(headers)},
        m_specialHeaders{specialHeaders},
        m_cookies{std::move(cookies)},
        m_body{std::move(body)} {}

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
      if(name == "Content-Length") {
        return Threading::With(m_contentLength,
          [&] (auto& contentLength) -> std::string& {
            if(contentLength.empty()) {
              contentLength = std::to_string(m_specialHeaders.m_contentLength);
            }
            return contentLength;
          });
      } else if(name == "Connection") {
        if(m_specialHeaders.m_connection == ConnectionHeader::KEEP_ALIVE) {
          static const std::string VALUE = "keep-alive";
          return VALUE;
        } else if(m_specialHeaders.m_connection == ConnectionHeader::CLOSE) {
          static const std::string VALUE = "close";
          return VALUE;
        } else {
          static const std::string VALUE = "Upgrade";
          return VALUE;
        }
      } else {
        return boost::none;
      }
    }
    return header->GetValue();
  }

  inline const std::vector<HttpHeader>& HttpServerRequest::GetHeaders() const {
    return m_headers;
  }

  inline const SpecialHeaders& HttpServerRequest::GetSpecialHeaders() const {
    return m_specialHeaders;
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
