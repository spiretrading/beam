#ifndef BEAM_PYTHON_WEB_SERVICES_HPP
#define BEAM_PYTHON_WEB_SERVICES_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the Cookie class.
   * @param module The module to export to.
   */
  void ExportCookie(pybind11::module& module);

  /**
   * Exports the HttpClient class.
   * @param module The module to export to.
   */
  void ExportHttpClient(pybind11::module& module);

  /**
   * Exports the HttpHeader class.
   * @param module The module to export to.
   */
  void ExportHttpHeader(pybind11::module& module);

  /**
   * Exports the HttpMethod enum.
   * @param module The module to export to.
   */
  void ExportHttpMethod(pybind11::module& module);

  /**
   * Exports the HttpRequest class.
   * @param module The module to export to.
   */
  void ExportHttpRequest(pybind11::module& module);

  /**
   * Exports the HttpRequestParser class.
   * @param module The module to export to.
   */
  void ExportHttpRequestParser(pybind11::module& module);

  /**
   * Exports the HttpResponse class.
   * @param module The module to export to.
   */
  void ExportHttpResponse(pybind11::module& module);

  /**
   * Exports the HttpResponseParser class.
   * @param module The module to export to.
   */
  void ExportHttpResponseParser(pybind11::module& module);

  /**
   * Exports the HttpStatusCode enum.
   * @param module The module to export to.
   */
  void ExportHttpStatusCode(pybind11::module& module);

  /**
   * Exports the HttpVersion enum.
   * @param module The module to export to.
   */
  void ExportHttpVersion(pybind11::module& module);

  /**
   * Exports the SecureSocketChannelFactory class.
   * @param module The module to export to.
   */
  void ExportSecureSocketChannelFactory(pybind11::module& module);

  /**
   * Exports the SocketChannelFactory class.
   * @param module The module to export to.
   */
  void ExportSocketChannelFactory(pybind11::module& module);

  /**
   * Exports the TcpSocketChannelFactory class.
   * @param module The module to export to.
   */
  void ExportTcpSocketChannelFactory(pybind11::module& module);

  /**
   * Exports the Uri class.
   * @param module The module to export to.
   */
  void ExportUri(pybind11::module& module);

  /**
   * Exports the WebServices namespace.
   * @param module The module to export to.
   */
  void ExportWebServices(pybind11::module& module);
}

#endif
