#ifndef BEAM_HTTP_RESPONSE_HPP
#define BEAM_HTTP_RESPONSE_HPP
#include <vector>
#include <boost/optional/optional.hpp>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpStatusCode.hpp"
#include "Beam/WebServices/HttpVersion.hpp"

namespace Beam {

  /** Handles the response to an HttpRequest. */
  class HttpResponse {
    public:

      /** Constructs an HttpResponse with a status of OK. */
      HttpResponse();

      /**
       * Constructs an HttpResponse.
       * @param status_code The HttpStatusCode.
       */
      explicit HttpResponse(HttpStatusCode status_code);

      /**
       * Constructs an HttpResponse.
       * @param version The HTTP version.
       * @param status_code The HttpStatusCode.
       * @param headers The response headers.
       * @param cookies The response Cookies.
       * @param body The body.
       */
      HttpResponse(HttpVersion version, HttpStatusCode status_code,
        std::vector<HttpHeader> headers, std::vector<Cookie> cookies,
        SharedBuffer body);

      /** Returns the version. */
      HttpVersion get_version() const;

      /** Sets the version. */
      void set_version(HttpVersion version);

      /** Returns the status code. */
      HttpStatusCode get_status_code() const;

      /** Sets the status code. */
      void set_status_code(HttpStatusCode status_code);

      /**
       * Returns the value of a header.
       * @param name The name of the header.
       * @return The value of the header with the specified name.
       */
      boost::optional<const std::string&>
        get_header(const std::string& name) const;

      /** Returns all headers. */
      const std::vector<HttpHeader>& get_headers() const;

      /**
       * Sets a header, adding it if it doesn't exist.
       * @param header The header to set.
       */
      void set_header(HttpHeader header);

      /** Returns all Cookies. */
      const std::vector<Cookie>& get_cookies() const;

      /**
       * Returns a Cookie with a specified name.
       * @param name The name of the Cookie.
       * @return The Cookie with the specified name or an empty Cookie if the
       *         Cookie was not found.
       */
      boost::optional<const Cookie&> get_cookie(const std::string& name) const;

      /**
       * Sets a Cookie, adding it if it doesn't exist.
       * @param cookie The Cookie to set.
       */
      void set_cookie(Cookie cookie);

      /** Returns the body. */
      const SharedBuffer& get_body() const;

      /**
       * Sets the body.
       * @param body The body to set.
       */
      void set_body(const SharedBuffer& body);

      /**
       * Outputs this response into a Buffer.
       * @param buffer The Buffer to output this response to.
       */
      template<IsBuffer B>
      void encode(Out<B> buffer) const;

    private:
      HttpVersion m_version;
      HttpStatusCode m_status_code;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      SharedBuffer m_body;
  };

  inline std::ostream& operator <<(
      std::ostream& sink, const HttpResponse& response) {
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    sink << buffer;
    return sink;
  }

  inline HttpResponse::HttpResponse()
    : HttpResponse(HttpStatusCode::OK) {}

  inline HttpResponse::HttpResponse(HttpStatusCode status_code)
      : m_version(HttpVersion::version_1_1()),
        m_status_code(status_code) {
    set_header(HttpHeader("Content-Length", "0"));
    set_header(HttpHeader("Connection", "keep-alive"));
  }

  inline HttpResponse::HttpResponse(HttpVersion version,
    HttpStatusCode status_code, std::vector<HttpHeader> headers,
    std::vector<Cookie> cookies, SharedBuffer body)
    : m_version(version),
      m_status_code(status_code),
      m_headers(std::move(headers)),
      m_cookies(std::move(cookies)),
      m_body(std::move(body)) {}

  inline HttpVersion HttpResponse::get_version() const {
    return m_version;
  }

  inline void HttpResponse::set_version(HttpVersion version) {
    m_version = version;
  }

  inline HttpStatusCode HttpResponse::get_status_code() const {
    return m_status_code;
  }

  inline void HttpResponse::set_status_code(HttpStatusCode status_code) {
    m_status_code = status_code;
  }

  inline boost::optional<const std::string&>
      HttpResponse::get_header(const std::string& name) const {
    auto header = std::find_if(m_headers.begin(), m_headers.end(),
      [&] (const auto& value) {
        return value.get_name() == name;
      });
    if(header == m_headers.end()) {
      return boost::none;
    }
    return header->get_value();
  }

  inline const std::vector<HttpHeader>& HttpResponse::get_headers() const {
    return m_headers;
  }

  inline void HttpResponse::set_header(HttpHeader header) {
    auto h = std::find_if(m_headers.begin(), m_headers.end(),
      [&] (const auto& value) {
        return value.get_name() == header.get_name();
      });
    if(h == m_headers.end()) {
      m_headers.push_back(std::move(header));
    } else {
      *h = std::move(header);
    }
  }

  inline const std::vector<Cookie>& HttpResponse::get_cookies() const {
    return m_cookies;
  }

  inline boost::optional<const Cookie&> HttpResponse::get_cookie(
      const std::string& name) const {
    auto cookie = std::find_if(m_cookies.begin(), m_cookies.end(),
      [&] (const auto& value) {
        return value.get_name() == name;
      });
    if(cookie == m_cookies.end()) {
      return boost::none;
    }
    return *cookie;
  }

  inline void HttpResponse::set_cookie(Cookie cookie) {
    auto c = std::find_if(m_cookies.begin(), m_cookies.end(),
      [&] (const auto& value) {
        return value.get_name() == cookie.get_name();
      });
    if(c == m_cookies.end()) {
      m_cookies.push_back(std::move(cookie));
    } else {
      *c = std::move(cookie);
    }
  }

  inline const SharedBuffer& HttpResponse::get_body() const {
    return m_body;
  }

  inline void HttpResponse::set_body(const SharedBuffer& body) {
    m_body = body;
    set_header(HttpHeader("Content-Length", std::to_string(m_body.get_size())));
  }

  template<IsBuffer B>
  void HttpResponse::encode(Out<B> buffer) const {
    auto conversion_buffer = std::array<char, 64>();
    auto buffer_output_stream = BufferOutputStream<B>(Ref(*buffer));
    buffer_output_stream << m_version;
    buffer_output_stream.flush();
    auto conversion_length = std::sprintf(
      conversion_buffer.data(), " %d ", static_cast<int>(m_status_code));
    append(*buffer, conversion_buffer.data(), conversion_length);
    auto& reason_phrase = get_reason_phrase(m_status_code);
    append(*buffer, reason_phrase.data(), reason_phrase.size());
    append(*buffer, "\r\n", 2);
    for(auto& header : m_headers) {
      buffer_output_stream << header << std::flush;
      append(*buffer, "\r\n", 2);
    }
    for(auto& cookie : m_cookies) {
      append(*buffer, "Set-Cookie: ", 12);
      buffer_output_stream << cookie << std::flush;
      append(*buffer, "\r\n", 2);
    }
    append(*buffer, "\r\n", 2);
    append(*buffer, m_body);
  }
}

#endif
