#include "Beam/WebServicesTests/HttpRequestParserTester.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Beam::WebServices::Tests;
using namespace boost;
using namespace std;

void HttpRequestParserTester::TestValidRequest() {
  HttpRequestParser parser;
  auto requestString =
    "GET /path/file.html HTTP/1.0\r\n"
    "From: someuser@jmarshall.com\r\n"
    "User-Agent: HTTPTool/1.0\r\n"
    "Cookie: theme=light; sessionToken=abc123\r\n"
    "\r\n";
  parser.Feed(requestString, strlen(requestString));
  auto request = parser.GetNextRequest();
  CPPUNIT_ASSERT(request.is_initialized());
  CPPUNIT_ASSERT(request->GetMethod() == HttpMethod::GET);
  CPPUNIT_ASSERT(request->GetUri().GetPath() == "/path/file.html");
  CPPUNIT_ASSERT(request->GetVersion() == HttpVersion::Version1_0());
  CPPUNIT_ASSERT(request->GetCookie("theme")->GetValue() == "light");
  CPPUNIT_ASSERT(request->GetCookie("sessionToken")->GetValue() == "abc123");
}

void HttpRequestParserTester::TestRequestHeaderCaseSensitivity() {
  HttpRequestParser parser;
  auto requestString =
    "GET /path/file.html HTTP/1.0\r\n"
    "host: 127.0.0.1\r\n"
    "user-AGENT: HTTPTool/1.0\r\n"
    "cookiE: theme=light; sessionToken=abc123\r\n"
    "\r\n";
  parser.Feed(requestString, strlen(requestString));
  auto request = parser.GetNextRequest();
  CPPUNIT_ASSERT(request.is_initialized());
  CPPUNIT_ASSERT(*request->GetHeader("Host") == "127.0.0.1");
  CPPUNIT_ASSERT(*request->GetHeader("User-Agent") == "HTTPTool/1.0");
  CPPUNIT_ASSERT(request->GetCookie("theme")->GetValue() == "light");
  CPPUNIT_ASSERT(request->GetCookie("sessionToken")->GetValue() == "abc123");
}
