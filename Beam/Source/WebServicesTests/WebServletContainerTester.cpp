#include "Avalon/WebServicesTests/WebServletContainerTester.hpp"
#include <boost/functional/factory.hpp>
#include "Avalon/IO/Buffer.hpp"
#include "Avalon/IO/Reader.hpp"
#include "Avalon/IO/Writer.hpp"
#include "Avalon/IOTests/MockClientChannel.hpp"
#include "Avalon/IOTests/MockServerConnection.hpp"
#include "Avalon/Serialization/JsonDeserializer.hpp"
#include "Avalon/Serialization/JsonSerializer.hpp"
#include "Avalon/Threading/TestTimer.hpp"
#include "Avalon/WebServices/HttpServer.hpp"
#include "Avalon/WebServices/HttpServerPredicates.hpp"
#include "Avalon/WebServices/HttpServerRequest.hpp"
#include "Avalon/WebServices/HttpServerResponse.hpp"

using namespace Avalon;
using namespace Avalon::IO;
using namespace Avalon::IO::Tests;
using namespace Avalon::Serialization;
using namespace Avalon::Services;
using namespace Avalon::SignalHandling;
using namespace Avalon::Threading;
using namespace Avalon::WebServices;
using namespace Avalon::WebServices::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

namespace Avalon {
  template<>
  struct Initializer<WebServletContainerTester::TestServlet> {};
}

WebServletContainerTester::TestChannel::TestChannel(Channel* channel)
    : m_channel(channel) {}

WebServletContainerTester::TestChannel::~TestChannel() {}

Connection& WebServletContainerTester::TestChannel::GetConnection() {
  return m_channel->GetConnection();
}

Reader& WebServletContainerTester::TestChannel::GetReader() {
  return m_channel->GetReader();
}

Writer& WebServletContainerTester::TestChannel::GetWriter() {
  return m_channel->GetWriter();
}

void WebServletContainerTester::TestServlet::Initialize(
    const Initializer<TestServlet>& initializer) {
  ServiceProtocolServlet<TestServlet, TestChannel>::Initialize(NullSlot(),
    bind(&TestServlet::RemoveChannel, this, _1));
  SetHandler<EchoService>(bind(&TestServlet::OnEchoRequest, this, _1, _2));
}

void WebServletContainerTester::TestServlet::OnEchoRequest(
    const RequestToken<EchoService>& request, int value) {
  request.SendResponse(value);
}

void WebServletContainerTester::setUp() {
  m_serverConnection = new MockServerConnection();
  m_container.reset(new WebServletContainer<TestServlet, TestChannel>(
    m_serverConnection, factory<TestTimer*>(), factory<TestTimer*>(),
    Initializer<TestServlet>(), bind(factory<JsonSerializer*>(), true),
    factory<JsonDeserializer*>()));
  m_container->SetServicePredicate<EchoService>(
    MatchesPath(HttpMethod::POST, "/echo"));
  m_container->Open();
  m_channel.reset(new MockClientChannel(m_serverConnection));
  CPPUNIT_ASSERT_NO_THROW(m_channel->GetConnection().Open());
}

void WebServletContainerTester::tearDown() {
  m_channel->GetConnection().Close();
  m_channel.reset();
  m_container->Close();
  m_container.reset();
}

void WebServletContainerTester::TestRequest() {

  // Send an initial request message, establishing the session.
  string body = "{\"value\":5}";
  string requestMessage =
"POST /echo HTTP/1.1\r\n"
"Content-Length: " + lexical_cast<string>(body.size()) + "\r\n"
"Host: www.test.com\r\n\r\n" + body;
  Async<void> writeResult;
  m_channel->GetWriter().Write(Buffer(requestMessage.c_str(),
    requestMessage.size()), Store(writeResult));
  writeResult.Wait(pos_infin);
  CPPUNIT_ASSERT_NO_THROW(writeResult.Get());
  writeResult.Reset();
  Buffer readBuffer;
  Async<int> readResult;
  m_channel->GetReader().Read(Store(readBuffer), Store(readResult));
  readResult.Wait(pos_infin);
  CPPUNIT_ASSERT_NO_THROW(readResult.Get());
}
