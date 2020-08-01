#include "Beam/Python/WebServices.hpp"
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <boost/lexical_cast.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/IO/VirtualChannel.hpp"
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
#include "Beam/WebServices/SessionDataStoreException.hpp"
#include "Beam/WebServices/SecureSocketChannelFactory.hpp"
#include "Beam/WebServices/SocketChannelFactory.hpp"
#include "Beam/WebServices/TcpChannelFactory.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::WebServices;
using namespace boost;
using namespace pybind11;

void Beam::Python::ExportCookie(pybind11::module& module) {
  class_<Cookie>(module, "Cookie")
    .def(init())
    .def(init<std::string, std::string>())
    .def_property_readonly("name", &Cookie::GetName)
    .def_property("value", &Cookie::GetValue, &Cookie::SetValue)
    .def_property("domain", &Cookie::GetDomain, &Cookie::SetDomain)
    .def_property("path", &Cookie::GetPath, &Cookie::SetPath)
    .def_property("is_secure", &Cookie::IsSecure, &Cookie::SetSecure)
    .def_property("is_http_only", &Cookie::IsHttpOnly, &Cookie::SetHttpOnly);
}

void Beam::Python::ExportHttpClient(pybind11::module& module) {
  using HttpClient = WebServices::HttpClient<std::unique_ptr<VirtualChannel>>;
  class_<HttpClient>(module, "HttpClient")
    .def(init(
      [] {
        return std::make_unique<HttpClient>(
          TcpSocketChannelFactory(Ref(*GetSocketThreadPool())));
      }))
    .def(init(
      [] (const IpAddress& interface) {
        return std::make_unique<HttpClient>(
          TcpSocketChannelFactory(interface, Ref(*GetSocketThreadPool())));
      }))
    .def("send", &HttpClient::Send, call_guard<GilRelease>());
}

void Beam::Python::ExportHttpHeader(pybind11::module& module) {
  class_<HttpHeader>(module, "HttpHeader")
    .def(init<const std::string&, const std::string&>())
    .def("__str__", &lexical_cast<std::string, HttpHeader>)
    .def_property_readonly("name", &HttpHeader::GetName)
    .def_property_readonly("value", &HttpHeader::GetValue);
}

void Beam::Python::ExportHttpMethod(pybind11::module& module) {
  enum_<HttpMethod>(module, "HttpMethod")
    .value("HEAD", HttpMethod::HEAD)
    .value("GET", HttpMethod::GET)
    .value("POST", HttpMethod::POST)
    .value("PUT", HttpMethod::PUT)
    .value("DELETE", HttpMethod::DELETE)
    .value("TRACE", HttpMethod::TRACE)
    .value("OPTIONS", HttpMethod::OPTIONS)
    .value("CONNECT", HttpMethod::CONNECT)
    .value("PATCH", HttpMethod::PATCH);
}

void Beam::Python::ExportHttpRequest(pybind11::module& module) {
  enum_<ConnectionHeader>(module, "ConnectionHeader")
    .value("CLOSE", ConnectionHeader::CLOSE)
    .value("KEEP_ALIVE", ConnectionHeader::KEEP_ALIVE)
    .value("UPGRADE", ConnectionHeader::UPGRADE);
  class_<SpecialHeaders>(module, "SpecialHeaders")
    .def(init())
    .def(init<HttpVersion>())
    .def_readwrite("host", &SpecialHeaders::m_host)
    .def_readwrite("content_length", &SpecialHeaders::m_contentLength)
    .def_readwrite("connection", &SpecialHeaders::m_connection);
  class_<HttpRequest>(module, "HttpRequest")
    .def(init<Uri>())
    .def(init<HttpMethod, Uri>())
    .def(init<HttpVersion, HttpMethod, Uri>())
    .def(init<HttpVersion, HttpMethod, Uri, std::vector<HttpHeader>,
      const SpecialHeaders&, std::vector<Cookie>, SharedBuffer>())
    .def("__str__", &lexical_cast<std::string, HttpRequest>)
    .def_property_readonly("version", &HttpRequest::GetVersion)
    .def_property_readonly("method", &HttpRequest::GetMethod)
    .def_property_readonly("uri", &HttpRequest::GetUri)
    .def("get_header", &HttpRequest::GetHeader)
    .def_property_readonly("headers", &HttpRequest::GetHeaders)
    .def_property_readonly("special_headers", &HttpRequest::GetSpecialHeaders)
    .def("add",
      static_cast<void (HttpRequest::*)(HttpHeader)>(&HttpRequest::Add))
    .def("get_cookie", &HttpRequest::GetCookie)
    .def_property_readonly("cookies", &HttpRequest::GetCookies)
    .def("add",
      static_cast<void (HttpRequest::*)(Cookie)>(&HttpRequest::Add))
    .def_property_readonly("body", &HttpRequest::GetBody,
      return_value_policy::reference_internal)
    .def("encode", &HttpRequest::Encode<SharedBuffer>);
}

void Beam::Python::ExportHttpRequestParser(pybind11::module& module) {
  class_<HttpRequestParser>(module, "HttpRequestParser")
    .def(init())
    .def("feed",
      [] (HttpRequestParser& self, const pybind11::str& value) {
        self.Feed(PyUnicode_AsUTF8(value.ptr()), len(value));
      })
    .def("feed",
      [] (HttpRequestParser& self, const SharedBuffer& value) {
        self.Feed(value.GetData(), value.GetSize());
      })
    .def("get_next_request", &HttpRequestParser::GetNextRequest);
}

void Beam::Python::ExportHttpResponse(pybind11::module& module) {
  class_<HttpResponse>(module, "HttpResponse")
    .def(init())
    .def(init<HttpStatusCode>())
    .def("__str__", &lexical_cast<std::string, HttpResponse>)
    .def_property("version", &HttpResponse::GetVersion,
      &HttpResponse::SetVersion)
    .def_property("status_code", &HttpResponse::GetStatusCode,
      &HttpResponse::SetStatusCode)
    .def("get_header", &HttpResponse::GetHeader)
    .def("headers", &HttpResponse::GetHeaders)
    .def("set_header", &HttpResponse::SetHeader)
    .def_property_readonly("cookies", &HttpResponse::GetCookies)
    .def("get_cookie", &HttpResponse::GetCookie)
    .def("set_cookie", &HttpResponse::SetCookie)
    .def_property("body", &HttpResponse::GetBody, &HttpResponse::SetBody)
    .def("encode", &HttpResponse::Encode<SharedBuffer>);
}

void Beam::Python::ExportHttpResponseParser(pybind11::module& module) {
  class_<HttpResponseParser>(module, "HttpResponseParser")
    .def(init())
    .def("feed",
      [] (HttpResponseParser& self, const pybind11::str& value) {
        self.Feed(PyUnicode_AsUTF8(value.ptr()), len(value));
      })
    .def("feed",
      [] (HttpResponseParser& self, const SharedBuffer& value) {
        self.Feed(value.GetData(), value.GetSize());
      })
    .def("get_next_response", &HttpResponseParser::GetNextResponse)
    .def("get_remaining_buffer", &HttpResponseParser::GetRemainingBuffer);
}

void Beam::Python::ExportHttpStatusCode(pybind11::module& module) {
  enum_<HttpStatusCode>(module, "HttpStatusCode")
   .value("CONTINUE", HttpStatusCode::CONTINUE)
   .value("SWITCHING_PROTOCOLS", HttpStatusCode::SWITCHING_PROTOCOLS)
   .value("PROCESSING", HttpStatusCode::PROCESSING)
   .value("OK", HttpStatusCode::OK)
   .value("CREATED", HttpStatusCode::CREATED)
   .value("ACCEPTED", HttpStatusCode::ACCEPTED)
   .value("NON_AUTHORITATIVE_INFORMATION",
      HttpStatusCode::NON_AUTHORITATIVE_INFORMATION)
   .value("NO_CONTENT", HttpStatusCode::NO_CONTENT)
   .value("RESET_CONTENT", HttpStatusCode::RESET_CONTENT)
   .value("PARTIAL_CONTENT", HttpStatusCode::PARTIAL_CONTENT)
   .value("MULTI_STATUS", HttpStatusCode::MULTI_STATUS)
   .value("MULTIPLE_CHOICES", HttpStatusCode::MULTIPLE_CHOICES)
   .value("MOVED_PERMANENTLY", HttpStatusCode::MOVED_PERMANENTLY)
   .value("FOUND", HttpStatusCode::FOUND)
   .value("SEE_OTHER", HttpStatusCode::SEE_OTHER)
   .value("NOT_MODIFIED", HttpStatusCode::NOT_MODIFIED)
   .value("USE_PROXY", HttpStatusCode::USE_PROXY)
   .value("SWITCH_PROXY", HttpStatusCode::SWITCH_PROXY)
   .value("TEMPORARY_REDIRECT", HttpStatusCode::TEMPORARY_REDIRECT)
   .value("BAD_REQUEST", HttpStatusCode::BAD_REQUEST)
   .value("UNAUTHORIZED", HttpStatusCode::UNAUTHORIZED)
   .value("PAYMENT_REQUIRED", HttpStatusCode::PAYMENT_REQUIRED)
   .value("FORBIDDEN", HttpStatusCode::FORBIDDEN)
   .value("NOT_FOUND", HttpStatusCode::NOT_FOUND)
   .value("METHOD_NOT_ALLOWED", HttpStatusCode::METHOD_NOT_ALLOWED)
   .value("NOT_ACCEPTABLE", HttpStatusCode::NOT_ACCEPTABLE)
   .value("PROXY_AUTHENTICATION_REQUIRED",
      HttpStatusCode::PROXY_AUTHENTICATION_REQUIRED)
   .value("REQUEST_TIMEOUT", HttpStatusCode::REQUEST_TIMEOUT)
   .value("CONFLICT", HttpStatusCode::CONFLICT)
   .value("GONE", HttpStatusCode::GONE)
   .value("LENGTH_REQUIRED", HttpStatusCode::LENGTH_REQUIRED)
   .value("PRECONDITION_FAILED", HttpStatusCode::PRECONDITION_FAILED)
   .value("REQUEST_ENTITY_TOO_LARGE", HttpStatusCode::REQUEST_ENTITY_TOO_LARGE)
   .value("REQUEST_URI_TOO_LONG", HttpStatusCode::REQUEST_URI_TOO_LONG)
   .value("UNSUPPORTED_MEDIA_TYPE", HttpStatusCode::UNSUPPORTED_MEDIA_TYPE)
   .value("REQUESTED_RANGE_NOT_SATISFIABLE",
      HttpStatusCode::REQUESTED_RANGE_NOT_SATISFIABLE)
   .value("EXPECTATION_FAILED", HttpStatusCode::EXPECTATION_FAILED)
   .value("IM_A_TEAPOT", HttpStatusCode::IM_A_TEAPOT)
   .value("UNPROCESSABLE_ENTITY", HttpStatusCode::UNPROCESSABLE_ENTITY)
   .value("LOCKED", HttpStatusCode::LOCKED)
   .value("FAILED_DEPENDENCY", HttpStatusCode::FAILED_DEPENDENCY)
   .value("UNORDERED_COLLECTION", HttpStatusCode::UNORDERED_COLLECTION)
   .value("UPGRADE_REQUIRED", HttpStatusCode::UPGRADE_REQUIRED)
   .value("NO_RESPONSE", HttpStatusCode::NO_RESPONSE)
   .value("RETRY_WITH", HttpStatusCode::RETRY_WITH)
   .value("BLOCK_BY_WINDOWS_PARENTAL_CONTROLS",
      HttpStatusCode::BLOCK_BY_WINDOWS_PARENTAL_CONTROLS)
   .value("CLIENT_CLOSED_REQUEST", HttpStatusCode::CLIENT_CLOSED_REQUEST)
   .value("INTERNAL_SERVER_ERROR", HttpStatusCode::INTERNAL_SERVER_ERROR)
   .value("NOT_IMPLEMENTED", HttpStatusCode::NOT_IMPLEMENTED)
   .value("BAD_GATEWAY", HttpStatusCode::BAD_GATEWAY)
   .value("SERVICE_UNAVAILABLE", HttpStatusCode::SERVICE_UNAVAILABLE)
   .value("GATEWAY_TIMEOUT", HttpStatusCode::GATEWAY_TIMEOUT)
   .value("HTTP_VERSION_NOT_SUPPORTED",
      HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED)
   .value("VARIANT_ALSO_NEGOTIATES", HttpStatusCode::VARIANT_ALSO_NEGOTIATES)
   .value("INSUFFICIENT_STORAGE", HttpStatusCode::INSUFFICIENT_STORAGE)
   .value("BANDWIDTH_LIMIT_EXCEEDED", HttpStatusCode::BANDWIDTH_LIMIT_EXCEEDED)
   .value("NOT_EXTENDED", HttpStatusCode::NOT_EXTENDED);
  module.def("get_reason_phrase", &GetReasonPhrase);
}

void Beam::Python::ExportHttpVersion(pybind11::module& module) {
  class_<HttpVersion>(module, "HttpVersion")
    .def(init())
    .def_property_readonly_static("V_1_0",
      [] (const object&) { return HttpVersion::Version1_0(); })
    .def_property_readonly_static("V_1_1",
      [] (const object&) { return HttpVersion::Version1_1(); })
    .def("__str__", &lexical_cast<std::string, HttpVersion>)
    .def_property_readonly("major", &HttpVersion::GetMajor)
    .def_property_readonly("minor", &HttpVersion::GetMinor)
    .def(self == self)
    .def(self != self);
}

void Beam::Python::ExportSecureSocketChannelFactory(pybind11::module& module) {
  class_<SecureSocketChannelFactory>(module, "SecureSocketChannelFactory")
    .def(init(
      [] {
        return std::make_unique<SecureSocketChannelFactory>(
          Ref(*GetSocketThreadPool()));
      }));
}

void Beam::Python::ExportSocketChannelFactory(pybind11::module& module) {
  class_<SocketChannelFactory>(module, "SocketChannelFactory")
    .def(init(
      [] {
        return std::make_unique<SocketChannelFactory>(
          Ref(*GetSocketThreadPool()));
      }));
}

void Beam::Python::ExportTcpSocketChannelFactory(pybind11::module& module) {
  class_<TcpSocketChannelFactory>(module, "TcpSocketChannelFactory")
    .def(init(
      [] {
        return std::make_unique<TcpSocketChannelFactory>(
          Ref(*GetSocketThreadPool()));
      }));
}

void Beam::Python::ExportUri(pybind11::module& module) {
  class_<Uri>(module, "Uri")
    .def(init())
    .def(init<const std::string&>())
    .def_property_readonly("scheme", &Uri::GetScheme)
    .def_property_readonly("username", &Uri::GetUsername)
    .def_property_readonly("password", &Uri::GetPassword)
    .def_property_readonly("hostname", &Uri::GetHostname)
    .def_property("port", &Uri::GetPort, &Uri::SetPort)
    .def_property_readonly("path", &Uri::GetPath)
    .def_property_readonly("query", &Uri::GetQuery)
    .def_property_readonly("fragment", &Uri::GetFragment);
}

void Beam::Python::ExportWebServices(pybind11::module& module) {
  auto submodule = module.def_submodule("web_services");
  ExportCookie(submodule);
  ExportHttpClient(submodule);
  ExportHttpHeader(submodule);
  ExportHttpMethod(submodule);
  ExportHttpRequest(submodule);
  ExportHttpRequestParser(submodule);
  ExportHttpResponse(submodule);
  ExportHttpResponseParser(submodule);
  ExportHttpStatusCode(submodule);
  ExportHttpVersion(submodule);
  ExportSecureSocketChannelFactory(submodule);
  ExportSocketChannelFactory(submodule);
  ExportTcpSocketChannelFactory(submodule);
  ExportUri(submodule);
  register_exception<InvalidHttpRequestException>(submodule,
    "InvalidHttpRequestException");
  register_exception<InvalidHttpResponseException>(submodule,
    "InvalidHttpResponseException");
  register_exception<MalformedUriException>(submodule, "MalformedUriException");
  register_exception<SessionDataStoreException>(submodule,
    "SessionDataStoreException", GetIOException().ptr());
}
