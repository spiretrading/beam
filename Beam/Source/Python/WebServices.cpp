#include "Beam/Python/WebServices.hpp"
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/PythonBindings.hpp"
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
#include "Beam/WebServices/Uri.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  HttpClient<std::unique_ptr<VirtualChannel>>* MakeHttpClient() {
    return new HttpClient<std::unique_ptr<VirtualChannel>>{
      TcpSocketChannelFactory{Ref(*GetSocketThreadPool())}};
  }

  HttpClient<std::unique_ptr<VirtualChannel>>* MakeInterfaceHttpClient(
      const IpAddress& interface) {
    return new HttpClient<std::unique_ptr<VirtualChannel>>{
      TcpSocketChannelFactory{interface, Ref(*GetSocketThreadPool())}};
  }

  HttpRequest* MakeFullHttpRequest(HttpVersion version, HttpMethod method,
      Uri uri, const boost::python::list& headers,
      const SpecialHeaders& specialHeaders, const boost::python::list& cookies,
      SharedBuffer body) {
    auto properHeaders = ToVector<HttpHeader>(headers);
    auto properCookies = ToVector<Cookie>(cookies);
    return new HttpRequest{version, method, std::move(uri),
      std::move(properHeaders), specialHeaders, std::move(properCookies),
      std::move(body)};
  }

  void EncodeHttpRequest(const HttpRequest& request, SharedBuffer& buffer) {
    request.Encode(Store(buffer));
  }

  void EncodeHttpResponse(const HttpResponse& response, SharedBuffer& buffer) {
    response.Encode(Store(buffer));
  }

  void HttpRequestParserFeedString(HttpRequestParser& parser,
      const boost::python::str& value) {
    parser.Feed(PyString_AsString(value.ptr()), len(value));
  }

  void HttpRequestParserFeedBuffer(HttpRequestParser& parser,
      const SharedBuffer& buffer) {
    parser.Feed(buffer.GetData(), buffer.GetSize());
  }

  void HttpResponseParserFeedString(HttpResponseParser& parser,
      const boost::python::str& value) {
    parser.Feed(PyString_AsString(value.ptr()), len(value));
  }

  void HttpResponseParserFeedBuffer(HttpResponseParser& parser,
      const SharedBuffer& buffer) {
    parser.Feed(buffer.GetData(), buffer.GetSize());
  }

  boost::python::object HttpRequestGetHeader(const HttpRequest& request,
      const string& name) {
    auto header = request.GetHeader(name);
    if(header.is_initialized()) {
      return boost::python::object{*header};
    }
    return boost::python::object{};
  }

  boost::python::object HttpRequestGetCookie(const HttpRequest& request,
      const string& name) {
    auto cookie = request.GetCookie(name);
    if(cookie.is_initialized()) {
      return boost::python::object{*cookie};
    }
    return boost::python::object{};
  }

  boost::python::object HttpResponseGetHeader(const HttpResponse& response,
      const string& name) {
    auto header = response.GetHeader(name);
    if(header.is_initialized()) {
      return boost::python::object{*header};
    }
    return boost::python::object{};
  }

  boost::python::object HttpResponseGetCookie(const HttpResponse& response,
      const string& name) {
    auto cookie = response.GetCookie(name);
    if(cookie.is_initialized()) {
      return boost::python::object{*cookie};
    }
    return boost::python::object{};
  }

  SecureSocketChannelFactory* MakeSecureSocketChannelFactory() {
    return new SecureSocketChannelFactory{Ref(*GetSocketThreadPool())};
  }

  SocketChannelFactory* MakeSocketChannelFactory() {
    return new SocketChannelFactory{Ref(*GetSocketThreadPool())};
  }

  TcpSocketChannelFactory* MakeTcpSocketChannelFactory() {
    return new TcpSocketChannelFactory{Ref(*GetSocketThreadPool())};
  }
}

void Beam::Python::ExportCookie() {
  class_<Cookie>("Cookie", init<>())
    .def(init<string, string>())
    .add_property("name", make_function(
      &Cookie::GetName, return_value_policy<copy_const_reference>()))
    .add_property("value", make_function(
      &Cookie::GetValue, return_value_policy<copy_const_reference>()),
      &Cookie::SetValue)
    .add_property("domain", make_function(
      &Cookie::GetDomain, return_value_policy<copy_const_reference>()),
      &Cookie::SetDomain)
    .add_property("path", make_function(
      &Cookie::GetPath, return_value_policy<copy_const_reference>()),
      &Cookie::SetPath)
    .add_property("is_secure", &Cookie::IsSecure, &Cookie::SetSecure)
    .add_property("is_http_only", &Cookie::IsSecure, &Cookie::IsHttpOnly);
}

void Beam::Python::ExportHttpClient() {
  using HttpClient = WebServices::HttpClient<std::unique_ptr<VirtualChannel>>;
  class_<HttpClient, noncopyable>("HttpClient", no_init)
    .def("__init__", make_constructor(&MakeHttpClient))
    .def("__init__", make_constructor(&MakeInterfaceHttpClient))
    .def("send", BlockingFunction(&HttpClient::Send));
}

void Beam::Python::ExportHttpHeader() {
  class_<HttpHeader>("HttpHeader", init<const string&, const string&>())
    .def("__str__", &lexical_cast<string, HttpHeader>)
    .add_property("name", make_function(&HttpHeader::GetName,
      return_value_policy<copy_const_reference>()))
    .add_property("value", make_function(&HttpHeader::GetValue,
      return_value_policy<copy_const_reference>()));
}

void Beam::Python::ExportHttpMethod() {
  enum_<HttpMethod>("HttpMethod")
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

void Beam::Python::ExportHttpRequest() {
  enum_<ConnectionHeader>("ConnectionHeader")
    .value("CLOSE", ConnectionHeader::CLOSE)
    .value("KEEP_ALIVE", ConnectionHeader::KEEP_ALIVE)
    .value("UPGRADE", ConnectionHeader::UPGRADE);
  class_<SpecialHeaders>("SpecialHeaders", init<>())
    .def(init<HttpVersion>())
    .def_readwrite("host", &SpecialHeaders::m_host)
    .def_readwrite("content_length", &SpecialHeaders::m_contentLength)
    .def_readwrite("connection", &SpecialHeaders::m_connection);
  class_<HttpRequest>("HttpRequest", init<Uri>())
    .def(init<HttpMethod, Uri>())
    .def(init<HttpVersion, HttpMethod, Uri>())
    .def("__init__", make_constructor(&MakeFullHttpRequest))
    .def("__str__", &lexical_cast<string, HttpRequest>)
    .add_property("version", make_function(&HttpRequest::GetVersion,
      return_value_policy<copy_const_reference>()))
    .add_property("method", &HttpRequest::GetMethod)
    .add_property("uri", make_function(&HttpRequest::GetUri,
      return_value_policy<copy_const_reference>()))
    .def("get_header", &HttpRequestGetHeader)
    .add_property("headers", make_function(&HttpRequest::GetHeaders,
      return_value_policy<copy_const_reference>()))
    .add_property("special_headers", make_function(
      &HttpRequest::GetSpecialHeaders,
      return_value_policy<copy_const_reference>()))
    .def("add",
      static_cast<void (HttpRequest::*)(HttpHeader)>(&HttpRequest::Add))
    .def("get_cookie", &HttpRequestGetCookie)
    .add_property("cookies", make_function(&HttpRequest::GetCookies,
      return_value_policy<copy_const_reference>()))
    .def("add",
      static_cast<void (HttpRequest::*)(Cookie)>(&HttpRequest::Add))
    .add_property("body", make_function(&HttpRequest::GetBody,
      return_internal_reference<>()))
    .def("encode", &EncodeHttpRequest);
  python_optional<HttpRequest>();
}

void Beam::Python::ExportHttpRequestParser() {
  class_<HttpRequestParser, noncopyable>("HttpRequestParser", init<>())
    .def("feed", &HttpRequestParserFeedString)
    .def("feed", &HttpRequestParserFeedBuffer)
    .def("get_next_request", &HttpRequestParser::GetNextRequest);
}

void Beam::Python::ExportHttpResponse() {
  class_<HttpResponse>("HttpResponse", init<>())
    .def(init<HttpStatusCode>())
    .def("__str__", &lexical_cast<string, HttpResponse>)
    .add_property("version", make_function(&HttpResponse::GetVersion,
      return_value_policy<copy_const_reference>()), &HttpResponse::SetVersion)
    .add_property("status_code", &HttpResponse::GetStatusCode,
      &HttpResponse::SetStatusCode)
    .def("get_header", &HttpResponseGetHeader)
    .def("headers", make_function(&HttpResponse::GetHeaders,
      return_value_policy<copy_const_reference>()))
    .def("set_header", &HttpResponse::SetHeader)
    .add_property("cookies", make_function(
      &HttpResponse::GetCookies, return_value_policy<copy_const_reference>()))
    .def("get_cookie", &HttpResponseGetCookie)
    .def("set_cookie", &HttpResponse::SetCookie)
    .add_property("body", make_function(
      &HttpResponse::GetBody, return_value_policy<copy_const_reference>()),
      &HttpResponse::SetBody)
    .def("encode", &EncodeHttpResponse);
  python_optional<HttpResponse>();
}

void Beam::Python::ExportHttpResponseParser() {
  class_<HttpResponseParser, noncopyable>("HttpResponseParser", init<>())
    .def("feed", &HttpResponseParserFeedString)
    .def("feed", &HttpResponseParserFeedBuffer)
    .def("get_next_response", &HttpResponseParser::GetNextResponse)
    .def("get_remaining_buffer", &HttpResponseParser::GetRemainingBuffer);
}

void Beam::Python::ExportHttpStatusCode() {
  enum_<HttpStatusCode>("HttpStatusCode")
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
  def("get_reason_phrase", &GetReasonPhrase,
    return_value_policy<copy_const_reference>());
}

void Beam::Python::ExportHttpVersion() {
  class_<HttpVersion>("HttpVersion", init<>())
    .add_static_property("V_1_0", &HttpVersion::Version1_0)
    .add_static_property("V_1_1", &HttpVersion::Version1_1)
    .def("__str__", &lexical_cast<string, HttpVersion>)
    .add_property("major", &HttpVersion::GetMajor)
    .add_property("minor", &HttpVersion::GetMinor)
    .def(self == self)
    .def(self != self);
}

void Beam::Python::ExportSecureSocketChannelFactory() {
  class_<SecureSocketChannelFactory>("SecureSocketChannelFactory", no_init)
    .def("__init__", make_constructor(&MakeSecureSocketChannelFactory));
}

void Beam::Python::ExportSocketChannelFactory() {
  class_<SocketChannelFactory>("SocketChannelFactory", no_init)
    .def("__init__", make_constructor(&MakeSocketChannelFactory));
}

void Beam::Python::ExportTcpSocketChannelFactory() {
  class_<TcpSocketChannelFactory>("TcpSocketChannelFactory", no_init)
    .def("__init__", make_constructor(&MakeTcpSocketChannelFactory));
}

void Beam::Python::ExportUri() {
  class_<Uri>("Uri", init<>())
    .def(init<const string&>())
    .add_property("scheme", make_function(&Uri::GetScheme,
      return_value_policy<copy_const_reference>()))
    .add_property("username", make_function(&Uri::GetUsername,
      return_value_policy<copy_const_reference>()))
    .add_property("password", make_function(&Uri::GetPassword,
      return_value_policy<copy_const_reference>()))
    .add_property("hostname", make_function(&Uri::GetHostname,
      return_value_policy<copy_const_reference>()))
    .add_property("port", &Uri::GetPort, &Uri::SetPort)
    .add_property("path", make_function(&Uri::GetPath,
      return_value_policy<copy_const_reference>()))
    .add_property("query", make_function(&Uri::GetQuery,
      return_value_policy<copy_const_reference>()))
    .add_property("fragment", make_function(&Uri::GetFragment,
      return_value_policy<copy_const_reference>()));
}

void Beam::Python::ExportWebServices() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".web_services");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("web_services") = nestedModule;
  scope parent = nestedModule;
  ExportCookie();
  ExportHttpClient();
  ExportHttpHeader();
  ExportHttpMethod();
  ExportHttpRequest();
  ExportHttpRequestParser();
  ExportHttpResponse();
  ExportHttpResponseParser();
  ExportHttpStatusCode();
  ExportHttpVersion();
  ExportSecureSocketChannelFactory();
  ExportSocketChannelFactory();
  ExportTcpSocketChannelFactory();
  ExportUri();
  ExportException<InvalidHttpRequestException, std::runtime_error>(
    "InvalidHttpRequestException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<InvalidHttpResponseException, std::runtime_error>(
    "InvalidHttpResponseException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<MalformedUriException, std::runtime_error>(
    "MalformedUriException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<SessionDataStoreException, IOException>(
    "SessionDataStoreException")
    .def(init<>())
    .def(init<const string&>());
}
