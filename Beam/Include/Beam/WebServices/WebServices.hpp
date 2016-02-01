#ifndef AVALON_WEBSERVICES_HPP
#define AVALON_WEBSERVICES_HPP
#include <boost/function.hpp>
#include "Avalon/Base/Base.hpp"

namespace Avalon {
namespace WebServices {
  class Cookie;
  class HttpProtocol;
  class HttpRequestParser;
  struct HttpRequestSlot;
  class HttpServer;
  class HttpServerChannel;
  class HttpServerRequest;
  class HttpServerResponse;
  class HttpSession;
  class HttpSessionHandler;
  struct HttpSessionRequestSlot;
  class InvalidHttpRequestException;
  class Uri;
  class WebServiceSession;
  template<typename ServletType, typename ChannelType>
    class WebServletContainer;

  //! Defines a rule for matching an HTTP request.
  /*!
    \param request The request to match.
    \return <code>true</code> iff the <i>request</i> matches this predicate.
  */
  typedef boost::function<bool (const HttpServerRequest*)> HttpRequestPredicate;
}
}

#endif // AVALON_WEBSERVICES_HPP
