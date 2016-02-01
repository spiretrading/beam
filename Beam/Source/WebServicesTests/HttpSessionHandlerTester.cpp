#include "Avalon/WebServicesTests/HttpSessionHandlerTester.hpp"
#include <boost/functional/factory.hpp>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/IO/Writer.hpp"
#include "Avalon/IOTests/MockServerConnection.hpp"
#include "Avalon/SignalHandling/NullSlot.hpp"
#include "Avalon/Threading/TestTimer.hpp"
#include "Avalon/WebServices/HttpServerPredicates.hpp"
#include "Avalon/WebServices/HttpServerRequest.hpp"
#include "Avalon/WebServices/HttpServerResponse.hpp"
#include "Avalon/WebServices/HttpSession.hpp"
#include "Avalon/WebServices/HttpSessionRequestSlot.hpp"

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

void HttpSessionHandlerTester::setUp() {
  m_serverConnection = new MockServerConnection();
  m_server.reset(new HttpServer());
  m_server->Initialize(m_serverConnection, factory<TestTimer*>(), NullSlot());
  CPPUNIT_ASSERT_NO_THROW(m_server->Open());
  m_sessionHandler.reset(new HttpSessionHandler());
  m_sessionHandler->Initialize(factory<HttpSession*>(), factory<TestTimer*>(),
    NullSlot(), NullSlot());
  m_channel.reset(new MockClientChannel(m_serverConnection));
  CPPUNIT_ASSERT_NO_THROW(m_channel->GetConnection().Open());
}

void HttpSessionHandlerTester::tearDown() {
  m_channel->GetConnection().Close();
  m_channel.reset();
  m_server.reset();
}

void HttpSessionHandlerTester::TestValidSession() {
  m_server->SetHandler(m_sessionHandler->GetSlot(&MatchAnyHttpRequest,
    m_sessionSink.GetSlot<HttpSessionRequestSlot::Slot>()));

  // Send an initial request message, establishing the session.
  string requestMessage =
"GET /test HTTP/1.1\r\n"
"Content-Length: 0\r\n"
"Host: www.test.com\r\n\r\n";
  Async<void> writeResult;
  m_channel->GetWriter().Write(Buffer(requestMessage.c_str(),
    requestMessage.size()), Store(writeResult));
  writeResult.Wait(pos_infin);
  CPPUNIT_ASSERT_NO_THROW(writeResult.Get());
  writeResult.Reset();
  SignalSink::SignalEntry entry = m_sessionSink.GetNextSignal(seconds(1));
  CPPUNIT_ASSERT(*entry.m_type == typeid(HttpSessionRequestSlot::Slot));
  HttpSession* session = any_cast<HttpSession*>(entry.m_parameters[0]);
  CPPUNIT_ASSERT(session != NULL);
  HttpServerRequest* request = any_cast<HttpServerRequest*>(
    entry.m_parameters[1]);
  HttpServerResponse* response = any_cast<HttpServerResponse*>(
    entry.m_parameters[2]);
  response->SendResponse();

  // Send a follow up message, using the session established above.
  string followUpMessage =
"GET /test HTTP/1.1\r\n"
"Content-Length: 0\r\n"
"Cookie: " + session->GetSessionCookie().ToString() + "\r\n"
"Host: www.test.com\r\n\r\n";
  m_channel->GetWriter().Write(Buffer(followUpMessage.c_str(),
    followUpMessage.size()), Store(writeResult));
  writeResult.Wait(pos_infin);
  CPPUNIT_ASSERT_NO_THROW(writeResult.Get());
  writeResult.Reset();
  entry = m_sessionSink.GetNextSignal(seconds(1));
  CPPUNIT_ASSERT(*entry.m_type == typeid(HttpSessionRequestSlot::Slot));
  HttpSession* followUpSession = any_cast<HttpSession*>(entry.m_parameters[0]);
  CPPUNIT_ASSERT(followUpSession == session);
  HttpServerRequest* followUpRequest = any_cast<HttpServerRequest*>(
    entry.m_parameters[1]);
  HttpServerResponse* followUpResponse = any_cast<HttpServerResponse*>(
    entry.m_parameters[2]);
  followUpResponse->SendResponse();
  session->Close();
  m_sessionHandler->Remove(session);
}
