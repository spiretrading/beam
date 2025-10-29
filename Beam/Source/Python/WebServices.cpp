#include "Beam/Python/WebServices.hpp"
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <boost/lexical_cast.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Collections.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/ToPythonChannel.hpp"
#include "Beam/Python/ToPythonConnection.hpp"
#include "Beam/Python/ToPythonEmailClient.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonWriter.hpp"
#include "Beam/Python/Utilities.hpp"
#include "Beam/WebServices/ContentTypePatterns.hpp"
#include "Beam/WebServices/Cookie.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailAddress.hpp"
#include "Beam/WebServices/HttpClient.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpMethod.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/HttpStatusCode.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/InvalidHttpRequestException.hpp"
#include "Beam/WebServices/InvalidHttpResponseException.hpp"
#include "Beam/WebServices/SecureSocketChannelFactory.hpp"
#include "Beam/WebServices/SmtpEmailClient.hpp"
#include "Beam/WebServices/SocketChannelFactory.hpp"
#include "Beam/WebServices/StandardOutEmailClient.hpp"
#include "Beam/WebServices/TcpChannelFactory.hpp"
#include "Beam/WebServices/TransferEncoding.hpp"
#include "Beam/WebServices/WebSessionDataStoreException.hpp"
#include "Beam/WebServices/WebSocket.hpp"
#include "Beam/WebServices/WebSocketChannel.hpp"
#include "Beam/WebServices/WebSocketConnection.hpp"
#include "Beam/WebServices/WebSocketReader.hpp"
#include "Beam/WebServices/WebSocketWriter.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

namespace {
  auto email_client = std::unique_ptr<class_<EmailClient>>();
}

class_<EmailClient>& Beam::Python::get_exported_email_client() {
  return *email_client;
}

void Beam::Python::export_content_type_patterns(module& module) {
  auto patterns = class_<ContentTypePatterns>(module, "ContentTypePatterns").
    def_static(
      "get_default_patterns", &ContentTypePatterns::get_default_patterns).
    def("get_content_type", &ContentTypePatterns::get_content_type).
    def("add_extension", &ContentTypePatterns::add_extension);
  export_default_methods(patterns);
}

void Beam::Python::export_cookie(module& module) {
  auto cookie = class_<Cookie>(module, "Cookie").
    def(pybind11::init<std::string, std::string>()).
    def_property_readonly("name", &Cookie::get_name).
    def_property("value", &Cookie::get_value, &Cookie::set_value).
    def_property("domain", &Cookie::get_domain, &Cookie::set_domain).
    def_property("path", &Cookie::get_path, &Cookie::set_path).
    def_property(
      "expiration", &Cookie::get_expiration, &Cookie::set_expiration).
    def_property("is_secure", &Cookie::is_secure, &Cookie::set_secure).
    def_property("is_http_only", &Cookie::is_http_only, &Cookie::set_http_only);
  export_default_methods(cookie);
}

void Beam::Python::export_email(module& module) {
  auto email = class_<Email>(module, "Email").
    def(pybind11::init<EmailAddress>()).
    def(pybind11::init<EmailAddress, EmailAddress>()).
    def_property("from", &Email::get_from, &Email::set_from).
    def_property("date", &Email::get_date, &Email::set_date).
    def_property_readonly("sender", &Email::get_sender).
    def("set_sender", &Email::set_sender).
    def_property_readonly("to", &Email::get_to).
    def("add_to", &Email::add_to).
    def_property("subject", &Email::get_subject, &Email::set_subject).
    def_property_readonly("bodies", &Email::get_bodies).
    def("add_body", overload_cast<std::string>(&Email::add_body)).
    def("add_body", overload_cast<std::string, std::string>(&Email::add_body)).
    def("find_header", &Email::find_header).
    def_property_readonly("additional_headers", &Email::get_additional_headers).
    def("set_header", &Email::set_header);
  export_default_methods(email);
  auto header = class_<Email::Header>(email, "Header").
    def_readwrite("name", &Email::Header::m_name).
    def_readwrite("value", &Email::Header::m_value);
  export_default_methods(header);
  auto body = class_<Email::Body>(email, "Body").
    def_readwrite("content_type", &Email::Body::m_content_type).
    def_readwrite("message", &Email::Body::m_message);
  export_default_methods(body);
}

void Beam::Python::export_email_address(module& module) {
  auto address = class_<EmailAddress>(module, "EmailAddress").
    def(pybind11::init<std::string>()).
    def(pybind11::init<std::string, std::string>()).
    def_property_readonly("address", &EmailAddress::get_address).
    def_property_readonly("display_name", &EmailAddress::get_display_name).
    def("get_user", &EmailAddress::get_user).
    def("get_domain", &EmailAddress::get_domain);
  export_default_methods(address);
}

void Beam::Python::export_http_client(module& module) {
  using HttpClient = HttpClient<std::unique_ptr<Channel>>;
  class_<HttpClient, std::shared_ptr<HttpClient>>(module, "HttpClient").
    def(pybind11::init([] {
      return make_python_shared<HttpClient>(TcpSocketChannelFactory());
    })).
    def(pybind11::init([] (const IpAddress& interface) {
      return make_python_shared<HttpClient>(TcpSocketChannelFactory(interface));
    })).
    def("send", &HttpClient::send, call_guard<GilRelease>());
}

void Beam::Python::export_http_header(module& module) {
  auto header = class_<HttpHeader>(module, "HttpHeader").
    def(pybind11::init<std::string, std::string>()).
    def_property_readonly("name", &HttpHeader::get_name).
    def_property_readonly("value", &HttpHeader::get_value);
  export_default_methods(header);
}

void Beam::Python::export_http_method(module& module) {
  enum_<HttpMethod::Type>(module, "HttpMethod").
    value("HEAD", HttpMethod::HEAD).
    value("GET", HttpMethod::GET).
    value("POST", HttpMethod::POST).
    value("PUT", HttpMethod::PUT).
    value("DELETE", HttpMethod::DELETE).
    value("TRACE", HttpMethod::TRACE).
    value("OPTIONS", HttpMethod::OPTIONS).
    value("CONNECT", HttpMethod::CONNECT).
    value("PATCH", HttpMethod::PATCH);
}

void Beam::Python::export_http_request(module& module) {
  enum_<ConnectionHeader>(module, "ConnectionHeader").
    value("CLOSE", ConnectionHeader::CLOSE).
    value("KEEP_ALIVE", ConnectionHeader::KEEP_ALIVE).
    value("UPGRADE", ConnectionHeader::UPGRADE);
  auto special_headers = class_<SpecialHeaders>(module, "SpecialHeaders").
    def(pybind11::init<HttpVersion>()).
    def_readwrite("host", &SpecialHeaders::m_host).
    def_readwrite("content_length", &SpecialHeaders::m_content_length).
    def_readwrite("connection", &SpecialHeaders::m_connection);
  export_default_methods(special_headers);
  auto request = class_<HttpRequest>(module, "HttpRequest").
    def(pybind11::init<Uri>()).
    def(pybind11::init<HttpMethod, Uri>()).
    def(pybind11::init<HttpMethod, Uri, SharedBuffer>()).
    def(pybind11::init<HttpVersion, HttpMethod, Uri>()).
    def(pybind11::init<HttpVersion, HttpMethod, Uri, std::vector<HttpHeader>,
      SpecialHeaders, std::vector<Cookie>, SharedBuffer>()).
    def_property_readonly("version", &HttpRequest::get_version).
    def_property_readonly("method", &HttpRequest::get_method).
    def_property_readonly("uri", &HttpRequest::get_uri).
    def("get_header", &HttpRequest::get_header).
    def_property_readonly("headers", &HttpRequest::get_headers).
    def_property_readonly("special_headers", &HttpRequest::get_special_headers).
    def("add", overload_cast<HttpHeader>(&HttpRequest::add)).
    def("get_cookie", &HttpRequest::get_cookie).
    def_property_readonly("cookies", &HttpRequest::get_cookies).
    def("add", overload_cast<Cookie>(&HttpRequest::add)).
    def_property_readonly(
      "body", &HttpRequest::get_body, return_value_policy::reference_internal).
    def("encode", &HttpRequest::encode<SharedBuffer>);
  export_default_methods(request);
}

void Beam::Python::export_http_request_parser(module& module) {
  auto parser = class_<HttpRequestParser>(module, "HttpRequestParser").
    def("feed", [] (HttpRequestParser& self, std::string_view value) {
      self.feed(value);
    }).
    def("get_next_request", &HttpRequestParser::get_next_request);
  export_default_methods(parser);
}

void Beam::Python::export_http_response(module& module) {
  auto response = class_<HttpResponse>(module, "HttpResponse").
    def(pybind11::init<HttpStatusCode>()).
    def(pybind11::init<HttpVersion, HttpStatusCode, std::vector<HttpHeader>,
      std::vector<Cookie>, SharedBuffer>()).
    def_property(
      "version", &HttpResponse::get_version, &HttpResponse::set_version).
    def_property("status_code", &HttpResponse::get_status_code,
      &HttpResponse::set_status_code).
    def("get_header", &HttpResponse::get_header).
    def_property_readonly("headers", &HttpResponse::get_headers).
    def("set_header", &HttpResponse::set_header).
    def_property_readonly("cookies", &HttpResponse::get_cookies).
    def("get_cookie", &HttpResponse::get_cookie).
    def("set_cookie", &HttpResponse::set_cookie).
    def_property("body", &HttpResponse::get_body, &HttpResponse::set_body).
    def("encode", &HttpResponse::encode<SharedBuffer>);
  export_default_methods(response);
}

void Beam::Python::export_http_response_parser(module& module) {
  auto parser = class_<HttpResponseParser>(module, "HttpResponseParser").
    def("feed", [] (HttpResponseParser& self, std::string_view value) {
      self.feed(value);
    }).
    def("get_next_response", &HttpResponseParser::get_next_response).
    def("get_remaining_buffer", &HttpResponseParser::get_remaining_buffer);
  export_default_methods(parser);
}

void Beam::Python::export_http_status_code(module& module) {
  enum_<HttpStatusCode>(module, "HttpStatusCode").
    value("CONTINUE", HttpStatusCode::CONTINUE).
    value("SWITCHING_PROTOCOLS", HttpStatusCode::SWITCHING_PROTOCOLS).
    value("PROCESSING", HttpStatusCode::PROCESSING).
    value("OK", HttpStatusCode::OK).
    value("CREATED", HttpStatusCode::CREATED).
    value("ACCEPTED", HttpStatusCode::ACCEPTED).
    value("NON_AUTHORITATIVE_INFORMATION",
      HttpStatusCode::NON_AUTHORITATIVE_INFORMATION).
    value("NO_CONTENT", HttpStatusCode::NO_CONTENT).
    value("RESET_CONTENT", HttpStatusCode::RESET_CONTENT).
    value("PARTIAL_CONTENT", HttpStatusCode::PARTIAL_CONTENT).
    value("MULTI_STATUS", HttpStatusCode::MULTI_STATUS).
    value("MULTIPLE_CHOICES", HttpStatusCode::MULTIPLE_CHOICES).
    value("MOVED_PERMANENTLY", HttpStatusCode::MOVED_PERMANENTLY).
    value("FOUND", HttpStatusCode::FOUND).
    value("SEE_OTHER", HttpStatusCode::SEE_OTHER).
    value("NOT_MODIFIED", HttpStatusCode::NOT_MODIFIED).
    value("USE_PROXY", HttpStatusCode::USE_PROXY).
    value("SWITCH_PROXY", HttpStatusCode::SWITCH_PROXY).
    value("TEMPORARY_REDIRECT", HttpStatusCode::TEMPORARY_REDIRECT).
    value("BAD_REQUEST", HttpStatusCode::BAD_REQUEST).
    value("UNAUTHORIZED", HttpStatusCode::UNAUTHORIZED).
    value("PAYMENT_REQUIRED", HttpStatusCode::PAYMENT_REQUIRED).
    value("FORBIDDEN", HttpStatusCode::FORBIDDEN).
    value("NOT_FOUND", HttpStatusCode::NOT_FOUND).
    value("METHOD_NOT_ALLOWED", HttpStatusCode::METHOD_NOT_ALLOWED).
    value("NOT_ACCEPTABLE", HttpStatusCode::NOT_ACCEPTABLE).
    value("PROXY_AUTHENTICATION_REQUIRED",
      HttpStatusCode::PROXY_AUTHENTICATION_REQUIRED).
    value("REQUEST_TIMEOUT", HttpStatusCode::REQUEST_TIMEOUT).
    value("CONFLICT", HttpStatusCode::CONFLICT).
    value("GONE", HttpStatusCode::GONE).
    value("LENGTH_REQUIRED", HttpStatusCode::LENGTH_REQUIRED).
    value("PRECONDITION_FAILED", HttpStatusCode::PRECONDITION_FAILED).
    value("REQUEST_ENTITY_TOO_LARGE", HttpStatusCode::REQUEST_ENTITY_TOO_LARGE).
    value("REQUEST_URI_TOO_LONG", HttpStatusCode::REQUEST_URI_TOO_LONG).
    value("UNSUPPORTED_MEDIA_TYPE", HttpStatusCode::UNSUPPORTED_MEDIA_TYPE).
    value("REQUESTED_RANGE_NOT_SATISFIABLE",
      HttpStatusCode::REQUESTED_RANGE_NOT_SATISFIABLE).
    value("EXPECTATION_FAILED", HttpStatusCode::EXPECTATION_FAILED).
    value("IM_A_TEAPOT", HttpStatusCode::IM_A_TEAPOT).
    value("UNPROCESSABLE_ENTITY", HttpStatusCode::UNPROCESSABLE_ENTITY).
    value("LOCKED", HttpStatusCode::LOCKED).
    value("FAILED_DEPENDENCY", HttpStatusCode::FAILED_DEPENDENCY).
    value("UNORDERED_COLLECTION", HttpStatusCode::UNORDERED_COLLECTION).
    value("UPGRADE_REQUIRED", HttpStatusCode::UPGRADE_REQUIRED).
    value("NO_RESPONSE", HttpStatusCode::NO_RESPONSE).
    value("RETRY_WITH", HttpStatusCode::RETRY_WITH).
    value("BLOCK_BY_WINDOWS_PARENTAL_CONTROLS",
      HttpStatusCode::BLOCK_BY_WINDOWS_PARENTAL_CONTROLS).
    value("CLIENT_CLOSED_REQUEST", HttpStatusCode::CLIENT_CLOSED_REQUEST).
    value("INTERNAL_SERVER_ERROR", HttpStatusCode::INTERNAL_SERVER_ERROR).
    value("NOT_IMPLEMENTED", HttpStatusCode::NOT_IMPLEMENTED).
    value("BAD_GATEWAY", HttpStatusCode::BAD_GATEWAY).
    value("SERVICE_UNAVAILABLE", HttpStatusCode::SERVICE_UNAVAILABLE).
    value("GATEWAY_TIMEOUT", HttpStatusCode::GATEWAY_TIMEOUT).
    value(
      "HTTP_VERSION_NOT_SUPPORTED", HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED).
    value("VARIANT_ALSO_NEGOTIATES", HttpStatusCode::VARIANT_ALSO_NEGOTIATES).
    value("INSUFFICIENT_STORAGE", HttpStatusCode::INSUFFICIENT_STORAGE).
    value("BANDWIDTH_LIMIT_EXCEEDED", HttpStatusCode::BANDWIDTH_LIMIT_EXCEEDED).
    value("NOT_EXTENDED", HttpStatusCode::NOT_EXTENDED);
  module.def("get_reason_phrase", &get_reason_phrase);
}

void Beam::Python::export_http_version(module& module) {
  auto version = class_<HttpVersion>(module, "HttpVersion").
    def_static("version_1_0", &HttpVersion::version_1_0).
    def_static("version_1_1", &HttpVersion::version_1_1).
    def_property_readonly("major", &HttpVersion::get_major).
    def_property_readonly("minor", &HttpVersion::get_minor);
  export_default_methods(version);
}

void Beam::Python::export_secure_socket_channel_factory(module& module) {
  auto factory =
    class_<SecureSocketChannelFactory>(module, "SecureSocketChannelFactory");
  export_default_methods(factory);
}

void Beam::Python::export_smtp_email_client(pybind11::module& module) {
  using SmtpEmailClient = SmtpEmailClient<std::unique_ptr<Channel>>;
  class_<ToPythonEmailClient<SmtpEmailClient>,
      std::shared_ptr<ToPythonEmailClient<SmtpEmailClient>>>(
        module, "SmtpEmailClient").
    def(pybind11::init([] (const IpAddress& address) {
      return make_python_shared<ToPythonEmailClient<SmtpEmailClient>>([=] {
        return std::make_unique<Channel>(
          std::in_place_type<TcpSocketChannel>, address);
      });
    })).
    def(pybind11::init(
      [] (const IpAddress& address, const IpAddress& interface) {
        return make_python_shared<ToPythonEmailClient<SmtpEmailClient>>([=] {
          return std::make_unique<Channel>(std::in_place_type<TcpSocketChannel>,
            address, interface);
        });
      })).
    def("set_credentials", [] (ToPythonEmailClient<SmtpEmailClient>& self,
        const std::string& username, const std::string& password) {
      return self->set_credentials(username, password);
    }).
    def("send", &ToPythonEmailClient<SmtpEmailClient>::send).
    def("close", &ToPythonEmailClient<SmtpEmailClient>::close);
}

void Beam::Python::export_socket_channel_factory(module& module) {
  auto factory = class_<SocketChannelFactory>(module, "SocketChannelFactory");
  export_default_methods(factory);
}

void Beam::Python::export_standard_out_email_client(pybind11::module& module) {
  class_<ToPythonEmailClient<StandardOutEmailClient>,
      std::shared_ptr<ToPythonEmailClient<StandardOutEmailClient>>>(
        module, "StandardOutEmailClient").
    def(pybind11::init(
      &make_python_shared<ToPythonEmailClient<StandardOutEmailClient>>)).
    def("send", &ToPythonEmailClient<StandardOutEmailClient>::send).
    def("close", &ToPythonEmailClient<StandardOutEmailClient>::close);
}

void Beam::Python::export_tcp_socket_channel_factory(module& module) {
  auto factory =
    class_<TcpSocketChannelFactory>(module, "TcpSocketChannelFactory");
  export_default_methods(factory);
}

void Beam::Python::export_transfer_encoding(pybind11::module& module) {
  enum_<TransferEncoding>(module, "TransferEncoding").
    value("NONE", TransferEncoding::NONE).
    value("CHUNKED", TransferEncoding::CHUNKED).
    value("COMPRESS", TransferEncoding::COMPRESS).
    value("DEFLATE", TransferEncoding::DEFLATE).
    value("GZIP", TransferEncoding::GZIP).
    value("IDENTITY", TransferEncoding::IDENTITY);
}

void Beam::Python::export_uri(module& module) {
  auto uri = class_<Uri>(module, "Uri").
    def(pybind11::init<std::string>()).
    def_property_readonly("scheme", &Uri::get_scheme).
    def_property_readonly("username", &Uri::get_username).
    def_property_readonly("password", &Uri::get_password).
    def_property_readonly("hostname", &Uri::get_hostname).
    def_property("port", &Uri::get_port, &Uri::set_port).
    def_property_readonly("path", &Uri::get_path).
    def_property_readonly("query", &Uri::get_query).
    def_property_readonly("fragment", &Uri::get_fragment);
  export_default_methods(uri);
  implicitly_convertible<std::string, Uri>();
}

void Beam::Python::export_web_services(module& module) {
  email_client = std::make_unique<class_<EmailClient>>(
    export_email_client<EmailClient>(module, "EmailClient"));
  export_content_type_patterns(module);
  export_cookie(module);
  export_email_address(module);
  export_email(module);
  export_http_client(module);
  export_http_header(module);
  export_http_method(module);
  export_http_request(module);
  export_http_request_parser(module);
  export_http_response(module);
  export_http_response_parser(module);
  export_http_status_code(module);
  export_http_version(module);
  export_secure_socket_channel_factory(module);
  export_smtp_email_client(module);
  export_socket_channel_factory(module);
  export_standard_out_email_client(module);
  export_tcp_socket_channel_factory(module);
  export_transfer_encoding(module);
  export_uri(module);
  export_web_socket(module);
  export_web_socket_channel(module);
  export_web_socket_config(module);
  export_web_socket_connection(module);
  export_web_socket_reader(module);
  export_web_socket_writer(module);
  register_exception<InvalidHttpRequestException>(
    module, "InvalidHttpRequestException");
  register_exception<InvalidHttpResponseException>(
    module, "InvalidHttpResponseException");
  register_exception<MalformedUriException>(module, "MalformedUriException");
  register_exception<WebSessionDataStoreException>(
    module, "WebSessionDataStoreException", get_io_exception());
}

void Beam::Python::export_web_socket(pybind11::module& module) {
  using WebSocket = WebSocket<std::unique_ptr<Channel>>;
  class_<WebSocket, std::shared_ptr<WebSocket>>(module, "WebSocket").
    def(pybind11::init([] (const WebSocketConfig& config) {
      return make_python_shared<WebSocket>(config, TcpSocketChannelFactory());
    }), call_guard<GilRelease>()).
    def_property_readonly("uri", &WebSocket::get_uri).
    def("read", &WebSocket::read, call_guard<GilRelease>()).
    def("write", &WebSocket::write<SharedBuffer>, call_guard<GilRelease>()).
    def("close", &WebSocket::close, call_guard<GilRelease>());
}

void Beam::Python::export_web_socket_channel(pybind11::module& module) {
  using WebSocket = WebSocket<std::unique_ptr<Channel>>;
  export_channel<ToPythonChannel<WebSocketChannel<std::unique_ptr<Channel>>>>(
    module, "WebSocketChannel").
    def(pybind11::init([] (const WebSocketConfig& config) {
      return std::make_unique<ToPythonChannel<WebSocketChannel<
        std::unique_ptr<Channel>>>>(config, TcpSocketChannelFactory());
    }), call_guard<GilRelease>()).
    def("get_socket", [] (
        ToPythonChannel<WebSocketChannel<std::unique_ptr<Channel>>>& self) ->
          WebSocket& {
      return self->get_socket();
    }, return_value_policy::reference_internal);
}

void Beam::Python::export_web_socket_config(pybind11::module& module) {
  auto config = class_<WebSocketConfig>(module, "WebSocketConfig").
    def("set_uri", &WebSocketConfig::set_uri).
    def("set_version", &WebSocketConfig::set_version).
    def("set_protocols", &WebSocketConfig::set_protocols).
    def("set_extensions", &WebSocketConfig::set_extensions);
  export_default_methods(config);
}

void Beam::Python::export_web_socket_connection(pybind11::module& module) {
  using WebSocket = WebSocket<std::unique_ptr<Channel>>;
  export_connection<ToPythonConnection<WebSocketConnection<WebSocket>>>(
    module, "WebSocketConnection");
}

void Beam::Python::export_web_socket_reader(pybind11::module& module) {
  using WebSocket = WebSocket<std::unique_ptr<Channel>>;
  export_reader<ToPythonReader<WebSocketReader<WebSocket>>>(
    module, "WebSocketReader");
}

void Beam::Python::export_web_socket_writer(pybind11::module& module) {
  using WebSocket = WebSocket<std::unique_ptr<Channel>>;
  export_writer<ToPythonWriter<WebSocketWriter<WebSocket>>>(
    module, "WebSocketWriter");
}
