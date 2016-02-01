#include "Avalon/WebServicesTests/HttpServerTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/IO/Writer.hpp"
#include "Avalon/IOTests/MockServerConnection.hpp"
#include "Avalon/SignalHandling/NullSlot.hpp"
#include "Avalon/Threading/TestTimer.hpp"
#include "Avalon/WebServices/HttpServer.hpp"
#include "Avalon/WebServices/HttpServerPredicates.hpp"
#include "Avalon/WebServices/HttpServerRequest.hpp"
#include "Avalon/WebServices/HttpServerResponse.hpp"

using namespace Avalon;
using namespace Avalon::IO;
using namespace Avalon::IO::Tests;
using namespace Avalon::SignalHandling;
using namespace Avalon::Threading;
using namespace Avalon::WebServices;
using namespace Avalon::WebServices::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

void HttpServerTester::setUp() {
  m_serverConnection = new MockServerConnection();
  m_server.reset(new HttpServer());
  m_server->Initialize(m_serverConnection, factory<TestTimer*>(), NullSlot());
  CPPUNIT_ASSERT_NO_THROW(m_server->Open());
  m_channel.reset(new MockClientChannel(m_serverConnection));
  CPPUNIT_ASSERT_NO_THROW(m_channel->GetConnection().Open());
}

void HttpServerTester::tearDown() {
  m_channel->GetConnection().Close();
  m_channel.reset();
  m_server.reset();
}

void HttpServerTester::TestHeaderInTwoWrites() {}

void HttpServerTester::TestRequestInTwoWrites() {
  string body = "Hello world.";
  string requestMessage =
"GET /test HTTP/1.1\r\n"
"Content-Length: " + lexical_cast<string>(body.size()) + "\r\n"
"Host: www.test.com\r\n\r\n";
  m_server->SetHandler(&MatchAnyHttpRequest,
    m_sessionSink.GetSlot<HttpRequestSlot::Slot>());
  Async<void> writeResult;
  m_channel->GetWriter().Write(Buffer(requestMessage.c_str(),
    requestMessage.size()), Store(writeResult));
  writeResult.Wait(pos_infin);
  CPPUNIT_ASSERT_NO_THROW(writeResult.Get());
  writeResult.Reset();
  m_channel->GetWriter().Write(Buffer(body.c_str(), body.size()),
    Store(writeResult));
  writeResult.Wait(pos_infin);
  CPPUNIT_ASSERT_NO_THROW(writeResult.Get());
  SignalSink::SignalEntry entry = m_sessionSink.GetNextSignal(seconds(1));
  CPPUNIT_ASSERT(*entry.m_type == typeid(HttpRequestSlot::Slot));
  HttpServerRequest* request = any_cast<HttpServerRequest*>(
    entry.m_parameters[0]);
  CPPUNIT_ASSERT(request->GetVersion() == make_tuple(1, 1));
  CPPUNIT_ASSERT(request->GetUri().ToString() == "/test");
  CPPUNIT_ASSERT(request->GetHeader("Host") == "www.test.com");
  string receivedBody(request->GetBody().GetData(),
    request->GetBody().GetSize());
  CPPUNIT_ASSERT(receivedBody == body);
  HttpServerResponse* response = any_cast<HttpServerResponse*>(
    entry.m_parameters[1]);
  response->SendResponse();
}
