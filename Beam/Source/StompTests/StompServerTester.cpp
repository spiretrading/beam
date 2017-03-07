#include "Beam/StompTests/StompServerTester.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Stomp;
using namespace Beam::Stomp::Tests;

void StompServerTester::setUp() {
  m_serverConnection.emplace();
  m_serverConnection->Open();
  m_clientChannel.emplace("stomp", Ref(*m_serverConnection));
  Routines::Spawn(
    [&] {
      m_clientChannel->GetConnection().Open();
    });
  auto serverChannel = m_serverConnection->Accept();
  m_server.emplace(std::move(serverChannel));
}

void StompServerTester::tearDown() {
  m_server.reset();
  m_clientChannel.reset();
  m_serverConnection.reset();
}

void StompServerTester::TestReceivingConnectCommand() {
  auto contents = BufferFromString<SharedBuffer>(
    "CONNECT\n"
    "accept-version:1.2\n"
    "host:testhost\n\n\n");
  m_clientChannel->GetWriter().Write(contents);
  m_server->Open();
  SharedBuffer responseBuffer;
  m_clientChannel->GetReader().Read(Store(responseBuffer));
  StompFrameParser parser;
  parser.Feed(responseBuffer.GetData(), responseBuffer.GetSize());
  auto response = parser.GetNextFrame();
  CPPUNIT_ASSERT(response.is_initialized());
  CPPUNIT_ASSERT(response->GetCommand() == StompCommand::CONNECTED);
  auto versionHeader = response->FindHeader("version");
  CPPUNIT_ASSERT(versionHeader.is_initialized());
  CPPUNIT_ASSERT(*versionHeader == "1.2");
}
