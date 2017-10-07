#ifndef BEAM_HTTPSERVERREQUEST_HPP
#define BEAM_HTTPSERVERREQUEST_HPP
#include <sstream>
#include <string>
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

    //! The host.
    std::string m_host;

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

  /*! \class HttpRequest
      \brief Represents an HTTP request.
   */
  class HttpRequest {
    public:

      //! Constructs an HTTP/1.1 GET request.
      /*!
        \param uri The URI to request.
      */
      HttpRequest(Uri uri);

      //! Constructs an HTTP/1.1 request.
      /*!
        \param method The HTTP method.
        \param uri The URI to request.
      */
      HttpRequest(HttpMethod method, Uri uri);

      //! Constructs an HTTP/1.1 request.
      /*!
        \param method The HTTP method.
        \param uri The URI to request.
        \param body The body.
      */
      HttpRequest(HttpMethod method, Uri uri, IO::SharedBuffer body);

      //! Constructs an HttpRequest.
      /*!
        \param version The HTTP version in major/minor format.
        \param method The HTTP method.
        \param uri The URI to perform the <i>method</i> on.
      */
      HttpRequest(HttpVersion version, HttpMethod method, Uri uri);

      //! Constructs an HttpRequest.
      /*!
        \param version The HTTP version in major/minor format.
        \param method The HTTP method.
        \param uri The URI to perform the <i>method</i> on.
        \param headers The request headers.
        \param specialHeaders The set of specially designated headers.
        \param cookies The Cookies.
        \param body The message body.
      */
      HttpRequest(HttpVersion version, HttpMethod method, Uri uri,
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

      //! Adds a header.
      /*!
        \param header The header to add.
      */
      void Add(HttpHeader header);

      //! Returns a Cookie with a specified name.
      /*!
        \param name The name of the Cookie.
        \return The Cookie with the specified name or an empty Cookie if the
                Cookie was not found.
      */
      boost::optional<const Cookie&> GetCookie(const std::string& name) const;

      //! Returns all Cookies.
      const std::vector<Cookie>& GetCookies() const;

      //! Adds a Cookie.
      /*!
        \param cookie The Cookie to add.
      */
      void Add(Cookie cookie);

      //! Returns the message body.
      const IO::SharedBuffer& GetBody() const;

      //! Outputs this response into a Buffer.
      /*!
        \param buffer The Buffer to output this response to.
      */
      template<typename Buffer>
      void Encode(Out<Buffer> buffer) const;

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
      const HttpRequest& request) {
    sink << request.GetMethod() << ' ';
    if(request.GetUri().GetPath().empty()) {
      sink << '/';
    } else {
      sink << request.GetUri().GetPath();
    }
    if(request.GetMethod() == HttpMethod::GET &&
        !request.GetUri().GetQuery().empty()) {
      sink << '?' << request.GetUri().GetQuery();
    }
    sink << ' ';
    sink << request.GetVersion() << "\r\n";
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
      sink << "\r\n";
    }
    sink << "Host: " << request.GetSpecialHeaders().m_host << "\r\n";
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

  inline HttpRequest::HttpRequest(Uri uri)
      : HttpRequest{HttpMethod::GET, std::move(uri)} {}

  inline HttpRequest::HttpRequest(HttpMethod method, Uri uri)
      : HttpRequest{HttpVersion::Version1_1(), method, std::move(uri)} {}

  inline HttpRequest::HttpRequest(HttpMethod method, Uri uri,
      IO::SharedBuffer body)
      : HttpRequest{HttpVersion::Version1_1(), method, std::move(uri), {}, {},
          {}, std::move(body)} {}

  inline HttpRequest::HttpRequest(HttpVersion version, HttpMethod method,
      Uri uri)
      : HttpRequest{version, method, std::move(uri), {}, {}, {}, {}} {}

  inline HttpRequest::HttpRequest(HttpVersion version, HttpMethod method,
      Uri uri, std::vector<HttpHeader> headers,
      const SpecialHeaders& specialHeaders, std::vector<Cookie> cookies,
      IO::SharedBuffer body)
      : m_version{std::move(version)},
        m_method{std::move(method)},
        m_uri{std::move(uri)},
        m_headers{std::move(headers)},
        m_specialHeaders{specialHeaders},
        m_cookies{std::move(cookies)},
        m_body{std::move(body)} {
    if(m_specialHeaders.m_host.empty()) {
      m_specialHeaders.m_host = m_uri.GetHostname();
      if(m_uri.GetPort() != 0 &&
          !((m_uri.GetScheme() == "http" || m_uri.GetScheme() == "ws") &&
          m_uri.GetPort() == 80) &&
          !((m_uri.GetScheme() == "https" || m_uri.GetScheme() == "wss") &&
          m_uri.GetPort() == 443)) {
        m_specialHeaders.m_host += ":" + std::to_string(m_uri.GetPort());
      }
    }
    if(m_method == HttpMethod::POST && !m_uri.GetQuery().empty()) {
      m_specialHeaders.m_contentLength = m_uri.GetQuery().size();
      m_body.Reset();
      m_body.Append(m_uri.GetQuery().c_str(), m_uri.GetQuery().size());
      Add(HttpHeader{"Content-Type", "application/x-www-form-urlencoded"});
    }
    if(!m_uri.GetUsername().empty() || !m_uri.GetPassword().empty()) {
      auto authentication = Base64Encode(IO::BufferFromString<IO::SharedBuffer>(
        m_uri.GetUsername() + ":" + m_uri.GetPassword()));
      Add(HttpHeader{"Authorization", "Basic " + authentication});
    }
    m_specialHeaders.m_contentLength = m_body.GetSize();
  }

  inline const HttpVersion& HttpRequest::GetVersion() const {
    return m_version;
  }

  inline HttpMethod HttpRequest::GetMethod() const {
    return m_method;
  }

  inline const Uri& HttpRequest::GetUri() const {
    return m_uri;
  }

  inline boost::optional<const std::string&> HttpRequest::GetHeader(
      const std::string& name) const {
    auto header = std::find_if(m_headers.begin(), m_headers.end(),
      [&] (const HttpHeader& value) {
        return value.GetName() == name;
      });
    if(header == m_headers.end()) {
      if(name == "Host") {
        return m_specialHeaders.m_host;
      } else if(name == "Content-Length") {
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

  inline const std::vector<HttpHeader>& HttpRequest::GetHeaders() const {
    return m_headers;
  }

  inline const SpecialHeaders& HttpRequest::GetSpecialHeaders() const {
    return m_specialHeaders;
  }

  inline void HttpRequest::Add(HttpHeader header) {
    if(header.GetName() == "Host") {
      m_specialHeaders.m_host = header.GetValue();
    } else if(header.GetName() == "Content-Length") {
      m_specialHeaders.m_contentLength = static_cast<std::size_t>(
        std::stoull(header.GetValue()));
      m_contentLength = std::string{};
    } else if(header.GetName() == "Connection") {
      if(header.GetValue() == "keep-alive") {
        m_specialHeaders.m_connection = ConnectionHeader::KEEP_ALIVE;
      } else if(header.GetValue() == "close") {
        m_specialHeaders.m_connection = ConnectionHeader::CLOSE;
      } else if(header.GetValue() == "Upgrade") {
        m_specialHeaders.m_connection = ConnectionHeader::UPGRADE;
      } else {
        BOOST_THROW_EXCEPTION(std::runtime_error{"Invalid Connection header."});
      }
    } else {
      m_headers.push_back(std::move(header));
    }
  }

  inline boost::optional<const Cookie&> HttpRequest::GetCookie(
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

  inline const std::vector<Cookie>& HttpRequest::GetCookies() const {
    return m_cookies;
  }

  inline void HttpRequest::Add(Cookie cookie) {
      m_cookies.push_back(std::move(cookie));
  }

  inline const IO::SharedBuffer& HttpRequest::GetBody() const {
    return m_body;
  }

  template<typename Buffer>
  void HttpRequest::Encode(Out<Buffer> buffer) const {
    std::stringstream ss;
    ss << *this;
    auto str = ss.str();
    buffer->Append(str.c_str(), str.size());
  }
}
}

#endif
