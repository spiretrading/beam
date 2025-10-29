#ifndef BEAM_PYTHON_WEB_SERVICES_HPP
#define BEAM_PYTHON_WEB_SERVICES_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/WebServices/EmailClient.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported EmailClient class. */
  BEAM_EXPORT_DLL pybind11::class_<EmailClient>& get_exported_email_client();

  /** Exports the ContentTypePatterns class. */
  void export_content_type_patterns(pybind11::module& module);

  /** Exports the Cookie class. */
  void export_cookie(pybind11::module& module);

  /** Exports the Email class. */
  void export_email(pybind11::module& module);

  /** Exports the EmailAddress class. */
  void export_email_address(pybind11::module& module);

  /** Exports an EmailClient class. */
  template<IsEmailClient C>
  auto export_email_client(pybind11::module& module, std::string_view name) {
    auto client_class = pybind11::class_<C>(module, name.data()).
      def("send", &C::send).
      def("close", &C::close).
      def("__repr__", [name = std::string(name)] (const C& self) {
        return "<" + name + ">";
      });
    if constexpr(!std::is_same_v<C, EmailClient>) {
      get_exported_email_client().def(
        pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<C, EmailClient>();
    }
    return client_class;
  }

  /** Exports the HttpClient class. */
  void export_http_client(pybind11::module& module);

  /** Exports the HttpHeader class. */
  void export_http_header(pybind11::module& module);

  /** Exports the HttpMethod enum. */
  void export_http_method(pybind11::module& module);

  /** Exports the HttpRequest class. */
  void export_http_request(pybind11::module& module);

  /** Exports the HttpRequestParser class. */
  void export_http_request_parser(pybind11::module& module);

  /** Exports the HttpResponse class. */
  void export_http_response(pybind11::module& module);

  /** Exports the HttpResponseParser class. */
  void export_http_response_parser(pybind11::module& module);

  /** Exports the HttpStatusCode enum. */
  void export_http_status_code(pybind11::module& module);

  /** Exports the HttpVersion enum. */
  void export_http_version(pybind11::module& module);

  /** Exports the SecureSocketChannelFactory class. */
  void export_secure_socket_channel_factory(pybind11::module& module);

  /** Exports the SmtpEmailClient class. */
  void export_smtp_email_client(pybind11::module& module);

  /** Exports the SocketChannelFactory class. */
  void export_socket_channel_factory(pybind11::module& module);

  /** Exports the StandardOutEmailClient class. */
  void export_standard_out_email_client(pybind11::module& module);

  /** Exports the TcpSocketChannelFactory class. */
  void export_tcp_socket_channel_factory(pybind11::module& module);

  /** Exports the TransferEncoding enum. */
  void export_transfer_encoding(pybind11::module& module);

  /** Exports the Uri class. */
  void export_uri(pybind11::module& module);

  /** Exports the WebServices namespace. */
  void export_web_services(pybind11::module& module);

  /** Exports the WebSocket class. */
  void export_web_socket(pybind11::module& module);

  /** Exports the WebSocketChannel class. */
  void export_web_socket_channel(pybind11::module& module);

  /** Exports the WebSocketConfig struct. */
  void export_web_socket_config(pybind11::module& module);

  /** Exports the WebSocketConnection class. */
  void export_web_socket_connection(pybind11::module& module);

  /** Exports the WebSocketReader class. */
  void export_web_socket_reader(pybind11::module& module);

  /** Exports the WebSocketWriter class. */
  void export_web_socket_writer(pybind11::module& module);
}

#endif
