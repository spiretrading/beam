#ifndef AVALON_HTTPSERVERREQUEST_HPP
#define AVALON_HTTPSERVERREQUEST_HPP
#include <map>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/WebServices/Cookie.hpp"
#include "Avalon/WebServices/HttpMethod.hpp"
#include "Avalon/WebServices/Uri.hpp"

namespace Avalon {
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
      HttpServerRequest(const boost::tuple<int, int>& version,
        HttpMethod method, const Uri& uri,
        const std::map<std::string, std::string>& headers,
        const std::vector<Cookie>& cookies, const IO::Buffer& body);

      ~HttpServerRequest();

      //! Returns the HTTP version in major/minor format.
      const boost::tuple<int, int>& GetVersion() const;

      //! Returns the HttpMethod to perform.
      HttpMethod GetMethod() const;

      //! Returns the URI.
      const Uri& GetUri() const;

      //! Returns the value of a header.
      /*!
        \param name The name of the header.
        \return The value of the header with the specified <i>name</i>.
      */
      const std::string& GetHeader(const std::string& name) const;

      //! Returns the headers.
      const std::map<std::string, std::string>& GetHeaders() const;

      //! Returns the Cookies.
      const std::vector<Cookie>& GetCookies() const;

      //! Returns a Cookie with a specified name.
      /*!
        \param name The name of the Cookie.
        \return The Cookie with the specified name or an empty Cookie if the
                Cookie was not found.
      */
      const Cookie& GetCookie(const std::string& name) const;

      //! Returns the message body.
      const IO::Buffer& GetBody() const;

    private:
      boost::tuple<int, int> m_version;
      HttpMethod m_method;
      Uri m_uri;
      std::map<std::string, std::string> m_headers;
      std::vector<Cookie> m_cookies;
      IO::Buffer m_body;
  };
}
}

#endif // AVALON_HTTPSERVERREQUEST_HPP
