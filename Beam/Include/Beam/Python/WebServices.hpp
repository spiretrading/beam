#ifndef BEAM_PYTHONWEBSERVICES_HPP
#define BEAM_PYTHONWEBSERVICES_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the Cookie class.
  void ExportCookie();

  //! Exports the HttpClient class.
  void ExportHttpClient();

  //! Exports the HttpHeader class.
  void ExportHttpHeader();

  //! Exports the HttpMethod enum.
  void ExportHttpMethod();

  //! Exports the HttpRequest class.
  void ExportHttpRequest();

  //! Exports the HttpRequestParser class.
  void ExportHttpRequestParser();

  //! Exports the HttpResponse class.
  void ExportHttpResponse();

  //! Exports the HttpResponseParser class.
  void ExportHttpResponseParser();

  //! Exports the HttpStatusCode enum.
  void ExportHttpStatusCode();

  //! Exports the HttpVersion enum.
  void ExportHttpVersion();

  //! Exports the SecureSocketChannelFactory class.
  void ExportSecureSocketChannelFactory();

  //! Exports the SocketChannelFactory class.
  void ExportSocketChannelFactory();

  //! Exports the TcpSocketChannelFactory class.
  void ExportTcpSocketChannelFactory();

  //! Exports the Uri class.
  void ExportUri();

  //! Exports the WebServices namespace.
  void ExportWebServices();
}
}

#endif
