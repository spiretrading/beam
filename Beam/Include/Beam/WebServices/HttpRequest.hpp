#ifndef BEAM_HTTP_SERVER_REQUEST_HPP
#define BEAM_HTTP_SERVER_REQUEST_HPP
#include <sstream>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpMethod.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {

  /** The set of acceptable Connection header values. */
  enum class ConnectionHeader {

    /** Close the connection. */
    CLOSE,

    /** Keep the connection open. */
    KEEP_ALIVE,

    /** Upgrade the connection. */
    UPGRADE
  };

  /** Keeps info about specially designated headers. */
  struct SpecialHeaders {

    /** The host. */
    std::string m_host;

    /** The size of the body. */
    std::size_t m_content_length;

    /** Whether to keep the connection open. */
    ConnectionHeader m_connection;

    /** Constructs a default SpecialHeaders field. */
    SpecialHeaders() noexcept;

    /**
     * Constructs a default SpecialHeaders field for a specified HTTP version.
     * @param version The HTTP version to get the default headers for.
     */
    explicit SpecialHeaders(HttpVersion version) noexcept;
  };

  /** Represents an HTTP request. */
  class HttpRequest {
    public:

      /**
       * Constructs an HTTP/1.1 GET request.
       * @param uri The URI to request.
       */
      explicit HttpRequest(Uri uri);

      /**
       * Constructs an HTTP/1.1 request.
       * @param method The HTTP method.
       * @param uri The URI to request.
       */
      HttpRequest(HttpMethod method, Uri uri);

      /**
       * Constructs an HTTP/1.1 request.
       * @param method The HTTP method.
       * @param uri The URI to request.
       * @param body The body.
       */
      HttpRequest(HttpMethod method, Uri uri, SharedBuffer body);

      /**
       * Constructs an HttpRequest.
       * @param version The HTTP version in major/minor format.
       * @param method The HTTP method.
       * @param uri The URI to perform the method on.
       */
      HttpRequest(HttpVersion version, HttpMethod method, Uri uri);

      /**
       * Constructs an HttpRequest.
       * @param version The HTTP version in major/minor format.
       * @param method The HTTP method.
       * @param uri The URI to perform the method on.
       * @param headers The request headers.
       * @param special_headers The set of specially designated headers.
       * @param cookies The Cookies.
       * @param body The message body.
       */
      HttpRequest(HttpVersion version, HttpMethod method, Uri uri,
        std::vector<HttpHeader> headers, SpecialHeaders special_headers,
        std::vector<Cookie> cookies, SharedBuffer body);

      /** Returns the HTTP version. */
      HttpVersion get_version() const;

      /** Returns the HttpMethod to perform. */
      HttpMethod get_method() const;

      /** Returns the URI. */
      const Uri& get_uri() const;

      /**
       * Returns the value of a header.
       * @param name The name of the header.
       * @return The value of the header with the specified name.
       */
      boost::optional<const std::string&>
        get_header(const std::string& name) const;

      /** Returns all headers. */
      const std::vector<HttpHeader>& get_headers() const;

      /** Returns the special headers. */
      const SpecialHeaders& get_special_headers() const;

      /**
       * Adds a header.
       * @param header The header to add.
       */
      void add(HttpHeader header);

      /**
       * Returns a Cookie with a specified name.
       * @param name The name of the Cookie.
       * @return The Cookie with the specified name or an empty Cookie if the
       *         Cookie was not found.
       */
      boost::optional<const Cookie&> get_cookie(const std::string& name) const;

      /** Returns all Cookies. */
      const std::vector<Cookie>& get_cookies() const;

      /**
       * Adds a Cookie.
       * @param cookie The Cookie to add.
       */
      void add(Cookie cookie);

      /** Returns the message body. */
      const SharedBuffer& get_body() const;

      /**
       * Outputs this response into a Buffer.
       * @param buffer The Buffer to output this response to.
       */
      template<IsBuffer B>
      void encode(Out<B> buffer) const;

    private:
      HttpVersion m_version;
      HttpMethod m_method;
      Uri m_uri;
      std::vector<HttpHeader> m_headers;
      SpecialHeaders m_special_headers;
      std::vector<Cookie> m_cookies;
      SharedBuffer m_body;
      mutable Sync<std::string> m_content_length;
  };

  inline std::ostream& operator <<(
      std::ostream& sink, ConnectionHeader connection_header) {
    if(connection_header == ConnectionHeader::CLOSE) {
      sink << "close";
    } else if(connection_header == ConnectionHeader::KEEP_ALIVE) {
      sink << "keep-alive";
    } else {
      sink << "Upgrade";
    }
    return sink;
  }

  inline std::ostream& operator <<(
      std::ostream& sink, const HttpRequest& request) {
    sink << request.get_method() << ' ';
    if(request.get_uri().get_path().empty()) {
      sink << '/';
    } else {
      sink << request.get_uri().get_path();
    }
    if(request.get_method() == HttpMethod::GET &&
        !request.get_uri().get_query().empty()) {
      sink << '?' << request.get_uri().get_query();
    }
    sink << ' ';
    sink << request.get_version() << "\r\n";
    for(auto& header : request.get_headers()) {
      sink << header.get_name() << ": " << header.get_value() << "\r\n";
    }
    if(!request.get_cookies().empty()) {
      sink << "Cookie: ";
      auto is_first = true;
      for(auto& cookie : request.get_cookies()) {
        if(!is_first) {
          sink << "; ";
        } else {
          is_first = false;
        }
        sink << cookie.get_name() << '=' << cookie.get_value();
      }
      sink << "\r\n";
    }
    sink << "Host: " << request.get_special_headers().m_host << "\r\n";
    sink << "Content-Length: " <<
      request.get_special_headers().m_content_length << "\r\n";
    sink << "Connection: " << request.get_special_headers().m_connection <<
      "\r\n";
    sink << "\r\n";
    sink << request.get_body();
    return sink;
  }

  inline SpecialHeaders::SpecialHeaders() noexcept
    : m_content_length(0),
      m_connection(ConnectionHeader::KEEP_ALIVE) {}

  inline SpecialHeaders::SpecialHeaders(HttpVersion version) noexcept {
    if(version == HttpVersion::version_1_1()) {
      m_content_length = 0;
      m_connection = ConnectionHeader::KEEP_ALIVE;
    } else {
      m_content_length = 0;
      m_connection = ConnectionHeader::CLOSE;
    }
  }

  inline HttpRequest::HttpRequest(Uri uri)
    : HttpRequest(HttpMethod::GET, std::move(uri)) {}

  inline HttpRequest::HttpRequest(HttpMethod method, Uri uri)
    : HttpRequest(HttpVersion::version_1_1(), method, std::move(uri)) {}

  inline HttpRequest::HttpRequest(
    HttpMethod method, Uri uri, SharedBuffer body)
    : HttpRequest(HttpVersion::version_1_1(), method, std::move(uri), {}, {},
        {}, std::move(body)) {}

  inline HttpRequest::HttpRequest(
    HttpVersion version, HttpMethod method, Uri uri)
    : HttpRequest(version, method, std::move(uri), {}, SpecialHeaders(version),
        {}, {}) {}

  inline HttpRequest::HttpRequest(HttpVersion version, HttpMethod method,
      Uri uri, std::vector<HttpHeader> headers, SpecialHeaders special_headers,
      std::vector<Cookie> cookies, SharedBuffer body)
      : m_version(version),
        m_method(method),
        m_uri(std::move(uri)),
        m_headers(std::move(headers)),
        m_special_headers(std::move(special_headers)),
        m_cookies(std::move(cookies)),
        m_body(std::move(body)) {
    if(m_special_headers.m_host.empty()) {
      m_special_headers.m_host = m_uri.get_hostname();
      if(m_uri.get_port() != 0 &&
          !((m_uri.get_scheme() == "http" || m_uri.get_scheme() == "ws") &&
            m_uri.get_port() == 80) &&
          !((m_uri.get_scheme() == "https" || m_uri.get_scheme() == "wss") &&
            m_uri.get_port() == 443)) {
        m_special_headers.m_host += ":" + std::to_string(m_uri.get_port());
      }
    }
    if(m_method == HttpMethod::POST && !m_uri.get_query().empty()) {
      m_special_headers.m_content_length = m_uri.get_query().size();
      reset(m_body);
      append(m_body, m_uri.get_query().c_str(), m_uri.get_query().size());
      add(HttpHeader("Content-Type", "application/x-www-form-urlencoded"));
    }
    if(!m_uri.get_username().empty() || !m_uri.get_password().empty()) {
      auto authentication = encode_base64(from<SharedBuffer>(
        m_uri.get_username() + ":" + m_uri.get_password()));
      add(HttpHeader("Authorization", "Basic " + authentication));
    }
    m_special_headers.m_content_length = m_body.get_size();
  }

  inline HttpVersion HttpRequest::get_version() const {
    return m_version;
  }

  inline HttpMethod HttpRequest::get_method() const {
    return m_method;
  }

  inline const Uri& HttpRequest::get_uri() const {
    return m_uri;
  }

  inline boost::optional<const std::string&>
      HttpRequest::get_header(const std::string& name) const {
    auto header = std::find_if(
      m_headers.begin(), m_headers.end(), [&] (const auto& value) {
        return value.get_name() == name;
      });
    if(header == m_headers.end()) {
      if(name == "Host") {
        return m_special_headers.m_host;
      } else if(name == "Content-Length") {
        return with(m_content_length, [&] (auto& content_length) -> auto& {
          if(content_length.empty()) {
            content_length = std::to_string(m_special_headers.m_content_length);
          }
          return content_length;
        });
      } else if(name == "Connection") {
        if(m_special_headers.m_connection == ConnectionHeader::KEEP_ALIVE) {
          static const auto VALUE = std::string("keep-alive");
          return VALUE;
        } else if(m_special_headers.m_connection == ConnectionHeader::CLOSE) {
          static const auto VALUE = std::string("close");
          return VALUE;
        } else {
          static const auto VALUE = std::string("Upgrade");
          return VALUE;
        }
      } else {
        return boost::none;
      }
    }
    return header->get_value();
  }

  inline const std::vector<HttpHeader>& HttpRequest::get_headers() const {
    return m_headers;
  }

  inline const SpecialHeaders& HttpRequest::get_special_headers() const {
    return m_special_headers;
  }

  inline void HttpRequest::add(HttpHeader header) {
    if(header.get_name() == "Host") {
      m_special_headers.m_host = header.get_value();
    } else if(header.get_name() == "Content-Length") {
      m_special_headers.m_content_length =
        static_cast<std::size_t>(std::stoull(header.get_value()));
      m_content_length = std::string();
    } else if(header.get_name() == "Connection") {
      if(header.get_value() == "keep-alive") {
        m_special_headers.m_connection = ConnectionHeader::KEEP_ALIVE;
      } else if(header.get_value() == "close") {
        m_special_headers.m_connection = ConnectionHeader::CLOSE;
      } else if(header.get_value() == "Upgrade") {
        m_special_headers.m_connection = ConnectionHeader::UPGRADE;
      } else {
        boost::throw_with_location(
          std::runtime_error("Invalid Connection header."));
      }
    } else {
      m_headers.push_back(std::move(header));
    }
  }

  inline boost::optional<const Cookie&>
      HttpRequest::get_cookie(const std::string& name) const {
    auto cookie = std::find_if(m_cookies.begin(), m_cookies.end(),
      [&] (const auto& value) {
        return value.get_name() == name;
      });
    if(cookie == m_cookies.end()) {
      return boost::none;
    }
    return *cookie;
  }

  inline const std::vector<Cookie>& HttpRequest::get_cookies() const {
    return m_cookies;
  }

  inline void HttpRequest::add(Cookie cookie) {
    m_cookies.push_back(std::move(cookie));
  }

  inline const SharedBuffer& HttpRequest::get_body() const {
    return m_body;
  }

  template<IsBuffer B>
  void HttpRequest::encode(Out<B> buffer) const {
    auto ss = std::stringstream();
    ss << *this;
    auto str = ss.str();
    append(*buffer, str.c_str(), str.size());
  }
}

#endif
