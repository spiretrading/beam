#ifndef AVALON_HTTPSERVERRESPONSE_HPP
#define AVALON_HTTPSERVERRESPONSE_HPP
#include <map>
#include <vector>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/WebServices/Cookie.hpp"
#include "Avalon/WebServices/HttpStatusCode.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpServerResponse
      \brief Handles the response to an HttpServerRequest.
   */
  class HttpServerResponse {
    public:

      //! Constructs an HttpServerResponse.
      /*!
        \param protocol The HttpProtocol from which the request was received.
        \param request The request being responded to.
      */
      HttpServerResponse(HttpProtocol* protocol, HttpServerRequest* request);

      ~HttpServerResponse();

      //! Returns the HttpProtocol from which the request was received.
      HttpProtocol& GetProtocol();

      //! Sets the status code.
      void SetStatus(HttpStatusCode statusCode);

      //! Sets the value of a header, adding it if it doesn't exist.
      /*!
        \param name The name of the header.
        \param value The header's value.
      */
      void SetHeader(const std::string& name, const std::string& value);

      //! Sets the body.
      void SetBody(const IO::Buffer& body);

      //! Adds a Cookie.
      /*!
        \param cookie The Cookie to add.
      */
      void AddCookie(const Cookie& cookie);

      //! Serializes this response into its raw representation.
      IO::Buffer Serialize();

      //! Sends the response and deletes this object.
      void SendResponse();

    private:
      HttpProtocol* m_protocol;
      boost::scoped_ptr<HttpServerRequest> m_request;
      HttpStatusCode m_statusCode;
      std::map<std::string, std::string> m_headers;
      std::vector<Cookie> m_cookies;
      IO::Buffer m_body;
  };
}
}

#endif // AVALON_HTTPSERVERRESPONSE_HPP
