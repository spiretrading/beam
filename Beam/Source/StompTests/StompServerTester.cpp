#include "Beam/StompTests/StompServerTester.hpp"

using namespace Beam;
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
  m_server->Open();
}

void StompServerTester::tearDown() {
  m_server.reset();
  m_clientChannel.reset();
  m_serverConnection.reset();
}

void StompServerTester::TestReceivingConnectCommand() {
}
