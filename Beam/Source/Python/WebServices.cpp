#include "Beam/Python/WebServices.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpMethod.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/Uri.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
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
    .def("__str__", &lexical_cast<string, HttpRequest>)
    .add_property("version", make_function(&HttpRequest::GetVersion,
      return_value_policy<copy_const_reference>()))
    .add_property("method", &HttpRequest::GetMethod)
    .add_property("uri", make_function(&HttpRequest::GetUri,
      return_value_policy<copy_const_reference>()))
    .def("get_header", &HttpRequest::GetHeader)
    .add_property("headers", make_function(&HttpRequest::GetHeaders,
      return_value_policy<copy_const_reference>()))
    .add_property("special_headers", make_function(
      &HttpRequest::GetSpecialHeaders,
      return_value_policy<copy_const_reference>()))
    .def("add", &HttpRequest::Add)
    .def("get_cookie", &HttpRequest::GetCookie)
    .add_property("cookies", make_function(&HttpRequest::GetCookies,
      return_value_policy<copy_const_reference>()))
    .add_property("body", make_function(&HttpRequest::GetBody,
      return_internal_reference<>()))
    .def("encode", &EncodeHttpRequest);
  python_optional<HttpRequest>();
}

void Beam::Python::ExportHttpRequestParser() {
  class_<HttpRequestParser>("HttpRequestParser", init<>())
    .def("feed", &HttpRequestParserFeedString)
    .def("feed", &HttpRequestParserFeedBuffer)
    .def("get_next_request", &HttpRequestParser::GetNextRequest);
}

void Beam::Python::ExportHttpResponse() {
  class_<HttpResponse>("HttpResponse", init<>())
    .def(init<HttpStatusCode>())
    .def("__str__", &lexical_cast<string, HttpResponse>)
    .add_property("status_code", &HttpResponse::GetStatusCode,
      &HttpResponse::SetStatusCode)
    .def("get_header", &HttpResponse::GetHeader)
    .def("headers", make_function(&HttpResponse::GetHeaders,
      return_value_policy<copy_const_reference>()))
    .def("set_header", &HttpResponse::SetHeader)
    .def("set_cookie", &HttpResponse::SetCookie)
    .def("set_body", &HttpResponse::SetBody)
    .def("encode", &EncodeHttpResponse);
  python_optional<HttpResponse>();
}

void Beam::Python::ExportHttpResponseParser() {
  class_<HttpResponseParser>("HttpResponseParser", init<>())
    .def("feed", &HttpResponseParserFeedString)
    .def("feed", &HttpResponseParserFeedBuffer)
    .def("get_next_response", &HttpResponseParser::GetNextResponse)
    .def("get_remaining_buffer", &HttpResponseParser::GetRemainingBuffer);
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
  ExportHttpHeader();
  ExportHttpMethod();
  ExportHttpRequest();
  ExportHttpRequestParser();
  ExportHttpResponse();
  ExportHttpResponseParser();
  ExportUri();
}
