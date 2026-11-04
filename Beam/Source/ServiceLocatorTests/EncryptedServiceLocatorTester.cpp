#include <doctest/doctest.h>
#include "Beam/IO/ChannelAdapterServerConnection.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;

namespace {
  struct Keys {
    std::string m_private_key;
    std::string m_public_key;
  };

  auto make_keys() {
    auto key = Details::AesGcmKey(
      EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "prime256v1"), &EVP_PKEY_free);
    if(!key) {
      throw std::runtime_error("Unable to generate test key.");
    }
    auto bio = Details::AesGcmBio(BIO_new(BIO_s_mem()), &BIO_free);
    Details::require_aes_gcm(static_cast<bool>(bio));
    auto encode = [&] {
      auto data = static_cast<char*>(nullptr);
      auto size = BIO_get_mem_data(bio.get(), &data);
      return std::string(data, static_cast<std::size_t>(size));
    };
    auto keys = Keys();
    Details::require_aes_gcm(PEM_write_bio_PrivateKey(
      bio.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr));
    keys.m_private_key = encode();
    Details::require_aes_gcm(BIO_reset(bio.get()));
    Details::require_aes_gcm(PEM_write_bio_PUBKEY(bio.get(), key.get()));
    keys.m_public_key = encode();
    return keys;
  }

  class Fixture {
    public:
      using ClientChannel = ServiceLocatorChannel<
        std::unique_ptr<LocalClientChannel>>;
      using ServerChannel = ServiceLocatorChannel<
        std::shared_ptr<Channel>>;
      using Server = ChannelAdapterServerConnection<
        ServerConnection, ServerChannel>;
      using Container = ServiceProtocolServletContainer<
        MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>, Server,
        BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<TriggerTimer>>;
      using Builder = ServiceProtocolClientBuilder<
        MessageProtocol<std::unique_ptr<ClientChannel>,
          BinarySender<SharedBuffer>>, TriggerTimer>;
      using Client = ProtocolServiceLocatorClient<Builder>;
      Keys m_keys;
      LocalServiceLocatorDataStore m_store;
      LocalServerConnection m_listener;
      Queue<std::shared_ptr<Channel>> m_accepted;
      std::optional<Container> m_container;

      Fixture()
          : m_keys(make_keys()) {
        m_container.emplace(init(&m_store),
          init(&m_listener, [=, this] (std::unique_ptr<Channel> channel) {
            auto transport = std::shared_ptr<Channel>(std::move(channel));
            auto encrypted = std::make_unique<ServerChannel>(
              init(transport, init(&transport->get_reader()),
                init(&transport->get_writer())),
              AesGcmServerConfig(m_keys.m_private_key));
            m_accepted.push(transport);
            return encrypted;
          }), [] { return std::make_shared<TriggerTimer>(); });
      }

      auto make_builder(const AesGcmClientConfig& encryption) {
        return Builder([=, this] {
          auto channel =
            std::make_unique<LocalClientChannel>("encrypted", m_listener);
          auto reader = &channel->get_reader();
          auto writer = &channel->get_writer();
          return std::make_unique<ClientChannel>(
            init(std::move(channel), init(reader), init(writer)), encryption);
        }, [] { return std::make_unique<TriggerTimer>(); });
      }

      auto make_client() {
        return std::make_unique<Client>("root", "",
          make_builder(AesGcmClientConfig(m_keys.m_public_key)));
      }
  };
}

TEST_SUITE("EncryptedServiceLocator") {
  TEST_CASE("login") {
    auto fixture = Fixture();
    auto root = fixture.make_client();
    REQUIRE(root->get_account().m_name == "root");
    auto account = root->make_account("user", "password",
      DirectoryEntry::STAR_DIRECTORY);
    auto user = Fixture::Client("user", "password", fixture.make_builder(
      AesGcmClientConfig(fixture.m_keys.m_public_key)));
    REQUIRE(user.get_account() == account);
    auto key = generate_encryption_key();
    auto resumed = Fixture::Client(user.get_encrypted_session_id(key), key,
      fixture.make_builder(AesGcmClientConfig(fixture.m_keys.m_public_key)));
    REQUIRE(resumed.get_account() == account);
    REQUIRE_THROWS_AS(Fixture::Client("user", "wrong", fixture.make_builder(
      AesGcmClientConfig(fixture.m_keys.m_public_key))),
      AuthenticationException);
  }

  TEST_CASE("large_messages") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto properties = JsonObject();
    properties["payload"] = std::string(70000, 'x');
    auto entry = client->add("large", properties);
    auto services = client->locate("large");
    REQUIRE(services.size() == 1);
    REQUIRE(services.front() == entry);
    REQUIRE(services.front().get_properties() == properties);
  }

  TEST_CASE("reconnect") {
    auto fixture = Fixture();
    auto client = fixture.make_client();
    auto first = fixture.m_accepted.pop();
    auto session = client->get_session_id();
    first->get_connection().close();
    auto second = fixture.m_accepted.pop();
    REQUIRE(second != first);
    REQUIRE(!client->load_all_accounts().empty());
    REQUIRE(client->get_account().m_name == "root");
    REQUIRE(client->get_session_id() != session);
  }

  TEST_CASE("server_key") {
    auto fixture = Fixture();
    auto keys = make_keys();
    REQUIRE_THROWS_AS(Fixture::Client("root", "", fixture.make_builder(
      AesGcmClientConfig(keys.m_public_key))), ConnectException);
    REQUIRE(!fixture.m_accepted.try_pop());
    auto client = fixture.make_client();
    REQUIRE(client->get_account().m_name == "root");
  }

}
