#ifndef AVALON_HTTPREQUESTPARSER_HPP
#define AVALON_HTTPREQUESTPARSER_HPP
#include <map>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/WebServices/HttpMethod.hpp"
#include "Avalon/WebServices/Uri.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpRequestParser
      \brief Parses an HTTP request.
   */
  class HttpRequestParser {
    public:

      //! Constructs an HttpRequestParser.
      HttpRequestParser();

      ~HttpRequestParser();

      //! Returns the Buffer storing the HTTP request.
      IO::Buffer& GetBuffer();

      //! Parses the remaining data stored by this parser.
      /*!
        \return The parsed HTTP request, or <code>NULL</code> iff the currently
                stored data buffer represents an incomplete request.
        \throw InvalidHttpRequestException if the request is invalid.
      */
      HttpServerRequest* Parse();

    private:
      IO::Buffer m_buffer;
      int m_readOffset;
      HttpMethod m_method;
      Uri m_uri;
      boost::tuple<int, int> m_version;
      std::map<std::string, std::string> m_headers;
      std::vector<Cookie> m_cookies;
      IO::Buffer m_body;
      int m_state;
  };
}
}

#endif // AVALON_HTTPREQUESTPARSER_HPP
