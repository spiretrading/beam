#ifndef BEAM_WEBSERVICES_HPP
#define BEAM_WEBSERVICES_HPP

namespace Beam {
namespace WebServices {
  class AuthenticatedSession;
  class ContentTypePatterns;
  class Cookie;
  class FileStore;
  template<typename ChannelType> class HttpClient;
  class HttpHeader;
  enum class HttpMethod;
  class HttpRequest;
  struct HttpRequestSlot;
  class HttpResponse;
  class HttpResponseParser;
  template<typename ServletType, typename ServerConnectionType>
    class HttpServletContainer;
  enum class HttpStatusCode;
  template<typename ChannelType> struct HttpUpgradeSlot;
  class HttpVersion;
  class InvalidHttpRequestException;
  class InvalidHttpResponseException;
  class MalformedUriException;
  class SecureSocketChannelFactory;
  class Session;
  template<typename SessionType> class SessionStore;
  class ServerWebSocketChannel;
  class SocketChannelFactory;
  enum class TransferEncoding;
  class Uri;
  template<typename ChannelType> class WebSocket;
  template<typename WebSocketType> class WebSocketChannel;
  template<typename WebSocketType> class WebSocketConnection;
  template<typename WebSocketType> class WebSocketReader;
  template<typename WebSocketType> class WebSocketWriter;
}
}

#endif
