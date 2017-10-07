#ifndef BEAM_WEBSERVICES_HPP
#define BEAM_WEBSERVICES_HPP

namespace Beam {
namespace WebServices {
  class AuthenticatedSession;
  class ContentTypePatterns;
  class Cookie;
  class Email;
  struct EmailClient;
  class EmailAddress;
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
  class MySqlSessionDataStore;
  class NullSessionDataStore;
  class SecureSocketChannelFactory;
  class Session;
  struct SessionDataStore;
  class SessionDataStoreException;
  template<typename SessionType, typename SessionDataStoreType>
    class SessionStore;
  class ServerWebSocketChannel;
  template<typename ChannelBuilderType> class SmtpEmailClient;
  class SocketChannelFactory;
  class StandardOutEmailClient;
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
