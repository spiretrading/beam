#include <doctest/doctest.h>
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Stomp/StompServer.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Stomp;

namespace {
  struct Fixture {
    using ServerConnection = LocalServerConnection<SharedBuffer>;
    using ServerChannel = LocalServerChannel<SharedBuffer>;
    using ClientChannel = LocalClientChannel<SharedBuffer>;
    using TestStompServer = StompServer<std::unique_ptr<ServerChannel>>;

    ServerConnection m_serverConnection;
    boost::optional<ClientChannel> m_clientChannel;
    boost::optional<TestStompServer> m_server;

    Fixture() {
      m_serverConnection.Open();
      m_clientChannel.emplace("stomp", Ref(m_serverConnection));
      Spawn(
        [&] {
          m_clientChannel->GetConnection().Open();
        });
      m_server.emplace(m_serverConnection.Accept());
    }
  };
}

TEST_SUITE("StompServer") {
  TEST_CASE_FIXTURE(Fixture, "receiving_connect_command") {
    auto contents = BufferFromString<SharedBuffer>(
      "CONNECT\n"
      "accept-version:1.2\n"
      "host:testhost\n\n\n");
    m_clientChannel->GetWriter().Write(contents);
    m_server->Open();
    auto responseBuffer = SharedBuffer();
    m_clientChannel->GetReader().Read(Store(responseBuffer));
    auto parser = StompFrameParser();
    parser.Feed(responseBuffer.GetData(), responseBuffer.GetSize());
    auto response = parser.GetNextFrame();
    REQUIRE(response.is_initialized());
    REQUIRE(response->GetCommand() == StompCommand::CONNECTED);
    auto versionHeader = response->FindHeader("version");
    REQUIRE(versionHeader.is_initialized());
    REQUIRE(*versionHeader == "1.2");
  }
}
