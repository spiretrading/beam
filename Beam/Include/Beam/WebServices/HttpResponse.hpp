#ifndef BEAM_HTTPRESPONSE_HPP
#define BEAM_HTTPRESPONSE_HPP
#include <vector>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpStatusCode.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpResponse
      \brief Handles the response to an HttpRequest.
   */
  class HttpResponse {
    public:

      //! Constructs an HttpResponse with a status of OK.
      HttpResponse();

      //! Constructs an HttpResponse.
      /*!
        \param statusCode The HttpStatusCode.
      */
      HttpResponse(HttpStatusCode statusCode);

      //! Constructs an HttpResponse.
      /*!
        \param version The HTTP version.
        \param statusCode The HttpStatusCode.
        \param headers The response headers.
        \param cookies The response Cookies.
        \param body The body.
      */
      HttpResponse(HttpVersion version, HttpStatusCode statusCode,
        std::vector<HttpHeader> headers, std::vector<Cookie> cookies,
        IO::SharedBuffer body);

      //! Returns the version.
      const HttpVersion& GetVersion() const;

      //! Sets the version.
      void SetVersion(const HttpVersion& version);

      //! Returns the status code.
      HttpStatusCode GetStatusCode() const;

      //! Sets the status code.
      void SetStatusCode(HttpStatusCode statusCode);

      //! Returns the value of a header.
      /*!
        \param name The name of the header.
        \return The value of the header with the specified <i>name</i>.
      */
      boost::optional<const std::string&> GetHeader(
        const std::string& name) const;

      //! Returns all headers.
      const std::vector<HttpHeader>& GetHeaders() const;

      //! Sets a header, adding it if it doesn't exist.
      /*!
        \param header The header to set.
      */
      void SetHeader(HttpHeader header);

      //! Returns all Cookies.
      const std::vector<Cookie>& GetCookies() const;

      //! Returns a Cookie with a specified name.
      /*!
        \param name The name of the Cookie.
        \return The Cookie with the specified name or an empty Cookie if the
                Cookie was not found.
      */
      boost::optional<const Cookie&> GetCookie(const std::string& name) const;

      //! Sets a Cookie, adding it if it doesn't exist.
      /*!
        \param cookie The Cookie to set.
      */
      void SetCookie(Cookie cookie);

      //! Returns the body.
      const IO::SharedBuffer& GetBody() const;

      //! Sets the body.
      void SetBody(IO::SharedBuffer body);

      //! Outputs this response into a Buffer.
      /*!
        \param buffer The Buffer to output this response to.
      */
      template<typename Buffer>
      void Encode(Out<Buffer> buffer) const;

    private:
      HttpVersion m_version;
      HttpStatusCode m_statusCode;
      std::vector<HttpHeader> m_headers;
      std::vector<Cookie> m_cookies;
      IO::SharedBuffer m_body;
  };

  inline std::ostream& operator <<(std::ostream& sink,
      const HttpResponse& response) {
    IO::SharedBuffer buffer;
    response.Encode(Store(buffer));
    sink << buffer;
    return sink;
  }

  inline HttpResponse::HttpResponse()
      : HttpResponse{HttpStatusCode::OK} {}

  inline HttpResponse::HttpResponse(HttpStatusCode statusCode)
      : m_version{HttpVersion::Version1_1()},
        m_statusCode{statusCode} {
    SetHeader({"Content-Length", "0"});
    SetHeader({"Connection", "keep-alive"});
  }

  inline HttpResponse::HttpResponse(HttpVersion version,
      HttpStatusCode statusCode, std::vector<HttpHeader> headers,
      std::vector<Cookie> cookies, IO::SharedBuffer body)
      : m_version{version},
        m_statusCode{statusCode},
        m_headers{std::move(headers)},
        m_cookies{std::move(cookies)},
        m_body{std::move(body)} {}

  inline const HttpVersion& HttpResponse::GetVersion() const {
    return m_version;
  }

  inline void HttpResponse::SetVersion(const HttpVersion& version) {
    m_version = version;
  }

  inline HttpStatusCode HttpResponse::GetStatusCode() const {
    return m_statusCode;
  }

  inline void HttpResponse::SetStatusCode(HttpStatusCode statusCode) {
    m_statusCode = statusCode;
  }

  inline boost::optional<const std::string&> HttpResponse::GetHeader(
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

  inline const std::vector<HttpHeader>& HttpResponse::GetHeaders() const {
    return m_headers;
  }

  inline void HttpResponse::SetHeader(HttpHeader header) {
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

  inline const std::vector<Cookie>& HttpResponse::GetCookies() const {
    return m_cookies;
  }

  inline boost::optional<const Cookie&> HttpResponse::GetCookie(
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

  inline void HttpResponse::SetCookie(Cookie cookie) {
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

  inline const IO::SharedBuffer& HttpResponse::GetBody() const {
    return m_body;
  }

  inline void HttpResponse::SetBody(IO::SharedBuffer body) {
    m_body = std::move(body);
    SetHeader({"Content-Length", std::to_string(m_body.GetSize())});
  }

  template<typename Buffer>
  void HttpResponse::Encode(Out<Buffer> buffer) const {
    char conversionBuffer[64];
    IO::BufferOutputStream<Buffer> bufferOutputStream{Ref(*buffer)};
    bufferOutputStream << m_version;
    bufferOutputStream.flush();
    auto conversionLength =
      std::sprintf(conversionBuffer, " %d ", static_cast<int>(m_statusCode));
    buffer->Append(conversionBuffer, conversionLength);
    auto& reasonPhrase = GetReasonPhrase(m_statusCode);
    buffer->Append(reasonPhrase.c_str(), reasonPhrase.size());
    buffer->Append("\r\n", 2);
    for(auto& header : m_headers) {
      bufferOutputStream << header << std::flush;
      buffer->Append("\r\n", 2);
    }
    for(auto& cookie : m_cookies) {
      buffer->Append("Set-Cookie: ", 12);
      bufferOutputStream << cookie << std::flush;
      buffer->Append("\r\n", 2);
    }
    buffer->Append("\r\n", 2);
    buffer->Append(m_body);
  }
}
}

#endif
