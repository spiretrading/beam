#include "Avalon/WebServicesTests/HttpRequestParserTester.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "Avalon/WebServices/HttpRequestParser.hpp"
#include "Avalon/WebServices/HttpServerRequest.hpp"

using namespace Avalon;
using namespace Avalon::IO;
using namespace Avalon::WebServices;
using namespace Avalon::WebServices::Tests;
using namespace boost;
using namespace std;

void HttpRequestParserTester::setUp() {}

void HttpRequestParserTester::tearDown() {}

void HttpRequestParserTester::TestParsingRequest() {
  string body = "Hello world.";
  string requestMessage =
"GET /test HTTP/1.1\r\n"
"Content-Length: " + lexical_cast<string>(body.size()) + "\r\n"
"Host: www.test.com\r\n\r\n";
  HttpRequestParser parser;
  parser.GetBuffer().Append(requestMessage.c_str(), requestMessage.size());
  parser.GetBuffer().Append(body.c_str(), body.size());
  scoped_ptr<HttpServerRequest> request(parser.Parse());
  CPPUNIT_ASSERT(request != NULL);
  CPPUNIT_ASSERT(request->GetVersion() == make_tuple(1, 1));
  CPPUNIT_ASSERT(request->GetUri().ToString() == "/test");
  CPPUNIT_ASSERT(request->GetHeader("Host") == "www.test.com");
  string receivedBody(request->GetBody().GetData(),
    request->GetBody().GetSize());
  CPPUNIT_ASSERT(receivedBody == body);
}

void HttpRequestParserTester::TestParsingHeaderInTwoWrites() {
  string fragmentA = "GET /test HTTP/1.1\r\n";
  string fragmentB =
"Content-Length: 0\r\n"
"Host: www.test.com\r\n\r\n";
  HttpRequestParser parser;
  parser.GetBuffer().Append(fragmentA.c_str(), fragmentA.size());
  scoped_ptr<HttpServerRequest> request(parser.Parse());
  CPPUNIT_ASSERT(request == NULL);
  parser.GetBuffer().Append(fragmentB.c_str(), fragmentB.size());
  request.reset(parser.Parse());
  CPPUNIT_ASSERT(request != NULL);
  CPPUNIT_ASSERT(request->GetVersion() == make_tuple(1, 1));
  CPPUNIT_ASSERT(request->GetUri().ToString() == "/test");
  CPPUNIT_ASSERT(request->GetHeader("Host") == "www.test.com");
}

void HttpRequestParserTester::TestParsingRequestInTwoWrites() {
  string body = "Hello world.";
  string requestMessage =
"GET /test HTTP/1.1\r\n"
"Content-Length: " + lexical_cast<string>(body.size()) + "\r\n"
"Host: www.test.com\r\n\r\n";
  HttpRequestParser parser;
  parser.GetBuffer().Append(requestMessage.c_str(), requestMessage.size());
  scoped_ptr<HttpServerRequest> request(parser.Parse());
  CPPUNIT_ASSERT(request == NULL);
  parser.GetBuffer().Append(body.c_str(), body.size());
  request.reset(parser.Parse());
  CPPUNIT_ASSERT(request != NULL);
  CPPUNIT_ASSERT(request->GetVersion() == make_tuple(1, 1));
  CPPUNIT_ASSERT(request->GetUri().ToString() == "/test");
  CPPUNIT_ASSERT(request->GetHeader("Host") == "www.test.com");
  string receivedBody(request->GetBody().GetData(),
    request->GetBody().GetSize());
  CPPUNIT_ASSERT(receivedBody == body);
}

void HttpRequestParserTester::TestParsingBodyInTwoWrites() {
  string bodyA = "Hello";
  string bodyB = " world.";
  string requestMessage =
"GET /test HTTP/1.1\r\n"
"Content-Length: " + lexical_cast<string>(bodyA.size() + bodyB.size()) + "\r\n"
"Host: www.test.com\r\n\r\n";
  HttpRequestParser parser;
  parser.GetBuffer().Append(requestMessage.c_str(), requestMessage.size());
  scoped_ptr<HttpServerRequest> request(parser.Parse());
  CPPUNIT_ASSERT(request == NULL);
  parser.GetBuffer().Append(bodyA.c_str(), bodyA.size());
  request.reset(parser.Parse());
  CPPUNIT_ASSERT(request == NULL);
  parser.GetBuffer().Append(bodyB.c_str(), bodyB.size());
  request.reset(parser.Parse());
  CPPUNIT_ASSERT(request != NULL);
  CPPUNIT_ASSERT(request->GetVersion() == make_tuple(1, 1));
  CPPUNIT_ASSERT(request->GetUri().ToString() == "/test");
  CPPUNIT_ASSERT(request->GetHeader("Host") == "www.test.com");
  string receivedBody(request->GetBody().GetData(),
    request->GetBody().GetSize());
  CPPUNIT_ASSERT(receivedBody == bodyA + bodyB);
}
