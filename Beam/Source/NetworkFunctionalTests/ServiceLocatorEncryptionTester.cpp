#include <boost/asio/ip/tcp.hpp>
#include <doctest/doctest.h>
#include "Beam/IO/ChannelAdapterServerConnection.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;

namespace {
  constexpr auto PRIVATE_KEY = R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQglWOQIPNZYrr3ez7w
MD5pqZ1MAkC4n8sko0y4+7rMCTehRANCAATIIaCwH1WHFWi3/bfB3OWVQmed7ijj
er5QB+8ikyklveHL3XBqaX58GY0ZtbLeDlHqMpGk7TfD4qAliecBCIFV
-----END PRIVATE KEY-----
)";
  constexpr auto PUBLIC_KEY = R"(-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEyCGgsB9VhxVot/23wdzllUJnne4o
43q+UAfvIpMpJb3hy91waml+fBmNGbWy3g5R6jKRpO03w+KgJYnnAQiBVQ==
-----END PUBLIC KEY-----
)";

  auto listen() {
    auto context = boost::asio::io_context();
    for(auto i = 0; i != 32; ++i) {
      auto reservation = boost::asio::ip::tcp::acceptor(context,
        boost::asio::ip::tcp::endpoint(
          boost::asio::ip::address_v4::loopback(), 0));
      auto address = IpAddress(
        "127.0.0.1", reservation.local_endpoint().port());
      reservation.close();
      try {
        return std::pair(address, std::make_unique<TcpServerSocket>(address));
      } catch(const ConnectException&) {}
    }
    throw std::runtime_error("Unable to bind test listener.");
  }

  class Fixture {
    public:
      using EncryptedChannel = ServiceLocatorChannel<
        std::shared_ptr<Channel>>;
      using Server = ChannelAdapterServerConnection<
        ServerConnection, EncryptedChannel>;
      using Container = ServiceProtocolServletContainer<
        MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>, Server,
        BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<TriggerTimer>>;
      IpAddress m_address;
      std::unique_ptr<TcpServerSocket> m_listener;
      LocalServiceLocatorDataStore m_store;
      Queue<std::shared_ptr<Channel>> m_accepted;
      std::optional<Container> m_container;

      Fixture() {
        auto [address, listener] = listen();
        m_address = address;
        m_listener = std::move(listener);
        m_container.emplace(init(&m_store),
          init(m_listener.get(),
            [=, this] (std::unique_ptr<Channel> channel) {
              auto transport = std::shared_ptr<Channel>(std::move(channel));
              auto encrypted = std::make_unique<EncryptedChannel>(
                init(transport, init(&transport->get_reader()),
                  init(&transport->get_writer())),
                AesGcmServerConfig(PRIVATE_KEY));
              m_accepted.push(transport);
              return encrypted;
            }), [] { return std::make_shared<TriggerTimer>(); });
      }
  };
}

TEST_SUITE("ServiceLocatorEncryption") {
  TEST_CASE("stalled_handshake") {
    auto fixture = Fixture();
    auto stalled = TcpSocketChannel(fixture.m_address);
    auto client = ApplicationServiceLocatorClient(
      "root", "", fixture.m_address, AesGcmClientConfig(PUBLIC_KEY));
    REQUIRE(client.get_account().m_name == "root");
    fixture.m_container->close();
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(stalled.get_reader().read(out(buffer)), IOException);
  }

  TEST_CASE("application_login") {
    auto fixture = Fixture();
    auto encryption = AesGcmClientConfig(PUBLIC_KEY);
    auto root = ApplicationServiceLocatorClient(
      "root", "", fixture.m_address, encryption);
    REQUIRE(root.get_account().m_name == "root");
    auto account = root.make_account(
      "user", "password", DirectoryEntry::STAR_DIRECTORY);
    auto config = ServiceLocatorClientConfig(
      fixture.m_address, "user", "password", encryption);
    auto client = ApplicationServiceLocatorClient(config);
    REQUIRE(client.get_account() == account);
    auto key = generate_encryption_key();
    auto resumed = ApplicationServiceLocatorClient(
      client.get_encrypted_session_id(key), key, fixture.m_address, encryption);
    REQUIRE(resumed.get_account() == account);
  }

  TEST_CASE("large_messages") {
    auto fixture = Fixture();
    auto client = ApplicationServiceLocatorClient(
      "root", "", fixture.m_address, AesGcmClientConfig(PUBLIC_KEY));
    auto properties = JsonObject();
    properties["payload"] = std::string(70000, 'x');
    auto entry = client.add("large", properties);
    auto services = client.locate("large");
    REQUIRE(services.size() == 1);
    REQUIRE(services.front() == entry);
    REQUIRE(services.front().get_properties() == properties);
  }

  TEST_CASE("reconnect") {
    auto fixture = Fixture();
    auto client = ApplicationServiceLocatorClient(
      "root", "", fixture.m_address, AesGcmClientConfig(PUBLIC_KEY));
    auto first = fixture.m_accepted.pop();
    auto session = client.get_session_id();
    first->get_connection().close();
    auto second = fixture.m_accepted.pop();
    REQUIRE(second != first);
    REQUIRE(!client.load_all_accounts().empty());
    REQUIRE(client.get_account().m_name == "root");
    REQUIRE(client.get_session_id() != session);
  }

  TEST_CASE("invalid_key") {
    auto fixture = Fixture();
    auto encryption = AesGcmClientConfig("invalid");
    auto is_session = false;
    SUBCASE("malformed") {}
    SUBCASE("missing") {
      encryption.m_public_key.clear();
    }
    SUBCASE("missing_session_key") {
      encryption.m_public_key.clear();
      is_session = true;
    }
    if(is_session) {
      REQUIRE_THROWS_AS(ApplicationServiceLocatorClient("session", 0U,
        fixture.m_address, encryption), ConnectException);
    } else {
      REQUIRE_THROWS_AS(ApplicationServiceLocatorClient("root", "",
        fixture.m_address, encryption), ConnectException);
    }
    auto client = ApplicationServiceLocatorClient(
      "root", "", fixture.m_address, AesGcmClientConfig(PUBLIC_KEY));
    REQUIRE(client.get_account().m_name == "root");
  }
}
