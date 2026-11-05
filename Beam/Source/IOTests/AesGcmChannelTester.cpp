#include <atomic>
#include <latch>
#include <mutex>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/IO/AesGcmChannel.hpp"
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/SizeDeclarativeReader.hpp"
#include "Beam/IO/SizeDeclarativeWriter.hpp"
#include "Beam/IO/WrapperChannel.hpp"

using namespace Beam;

namespace {
  struct KeyPair {
    std::string m_private_key;
    std::string m_public_key;
  };

  auto pem(BIO& bio) {
    auto data = static_cast<char*>(nullptr);
    auto size = BIO_get_mem_data(&bio, &data);
    return std::string(data, static_cast<std::size_t>(size));
  }

  auto make_keys(std::string_view algorithm) {
    auto key = Details::AesGcmKey(nullptr, &EVP_PKEY_free);
    if(algorithm == "RSA") {
      key.reset(EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", std::size_t(2048)));
    } else if(algorithm == "ED25519") {
      key.reset(EVP_PKEY_Q_keygen(nullptr, nullptr, "ED25519"));
    } else {
      key.reset(EVP_PKEY_Q_keygen(nullptr, nullptr, "EC", "prime256v1"));
    }
    REQUIRE(key);
    auto output = KeyPair();
    auto bio = Details::AesGcmBio(BIO_new(BIO_s_mem()), &BIO_free);
    REQUIRE(PEM_write_bio_PrivateKey(
      bio.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr) == 1);
    output.m_private_key = pem(*bio);
    REQUIRE(BIO_reset(bio.get()) == 1);
    REQUIRE(PEM_write_bio_PUBKEY(bio.get(), key.get()) == 1);
    output.m_public_key = pem(*bio);
    return output;
  }

  const KeyPair& get_keys() {
    static auto keys = make_keys("EC");
    return keys;
  }

  class FragmentedReader {
    public:
      PipedReader* m_source;

      bool poll() const {
        return m_source->poll();
      }

      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size) {
        return m_source->read(out(destination), std::min(size, std::size_t(7)));
      }
  };

  class RecordingWriter {
    public:
      PipedWriter* m_destination;
      SharedBuffer m_output;
      bool m_is_tampered;
      bool m_is_failed;

      explicit RecordingWriter(PipedWriter& destination)
        : m_destination(&destination),
          m_is_tampered(false),
          m_is_failed(false) {}

      template<IsConstBuffer B>
      void write(const B& buffer) {
        if(m_is_failed) {
          throw IOException("Test write failure.");
        }
        auto output = SharedBuffer(buffer.get_data(), buffer.get_size());
        if(m_is_tampered && output.get_size() != 0) {
          output.get_mutable_data()[output.get_size() - 1] ^= 1;
        }
        append(m_output, output);
        m_destination->write(output);
      }
  };

  class TestConnection {
    public:
      PipedWriter* m_outgoing;
      PipedWriter* m_incoming;
      std::atomic_bool m_is_closed;

      TestConnection(PipedWriter& outgoing, PipedWriter& incoming)
        : m_outgoing(&outgoing),
          m_incoming(&incoming),
          m_is_closed(false) {}

      void close() {
        m_is_closed = true;
        m_outgoing->close();
        m_incoming->close();
      }
  };

  class TestTransport {
    public:
      using Identifier = NamedChannelIdentifier;
      using Connection = TestConnection;
      using Reader = FragmentedReader;
      using Writer = RecordingWriter;
      Identifier m_identifier;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      TestTransport(PipedReader& reader, PipedWriter& outgoing,
        PipedWriter& incoming)
        : m_identifier("test"),
          m_connection(outgoing, incoming),
          m_reader(&reader),
          m_writer(outgoing) {}

      const Identifier& get_identifier() const {
        return m_identifier;
      }

      Connection& get_connection() {
        return m_connection;
      }

      Reader& get_reader() {
        return m_reader;
      }

      Writer& get_writer() {
        return m_writer;
      }
  };

  class Session {
    public:
      using FramedTransport = WrapperChannel<TestTransport*,
        std::unique_ptr<SizeDeclarativeReader<FragmentedReader*>>,
        std::unique_ptr<SizeDeclarativeWriter<RecordingWriter*>>>;

      PipedReader m_client_input;
      PipedReader m_server_input;
      PipedWriter m_client_output;
      PipedWriter m_server_output;
      TestTransport m_client_transport;
      TestTransport m_server_transport;
      FramedTransport m_framed_client;
      FramedTransport m_framed_server;
      std::unique_ptr<AesGcmChannel<FramedTransport*>> m_client;
      std::unique_ptr<AesGcmChannel<FramedTransport*>> m_server;

      Session()
        : m_client_output(Ref(m_server_input)),
          m_server_output(Ref(m_client_input)),
          m_client_transport(
            m_client_input, m_client_output, m_server_output),
          m_server_transport(
            m_server_input, m_server_output, m_client_output),
          m_framed_client(&m_client_transport,
            std::make_unique<SizeDeclarativeReader<FragmentedReader*>>(
              &m_client_transport.m_reader),
            std::make_unique<SizeDeclarativeWriter<RecordingWriter*>>(
              &m_client_transport.m_writer)),
          m_framed_server(&m_server_transport,
            std::make_unique<SizeDeclarativeReader<FragmentedReader*>>(
              &m_server_transport.m_reader),
            std::make_unique<SizeDeclarativeWriter<RecordingWriter*>>(
              &m_server_transport.m_writer)) {}

      auto connect(const AesGcmClientConfig& client_config,
          const AesGcmServerConfig& server_config) {
        auto client_error = std::exception_ptr();
        auto server_error = std::exception_ptr();
        auto server = std::jthread([&] {
          try {
            m_server = std::make_unique<AesGcmChannel<FramedTransport*>>(
              &m_framed_server, server_config);
            m_server->get_writer().write(SharedBuffer());
          } catch(...) {
            server_error = std::current_exception();
          }
        });
        try {
          m_client = std::make_unique<AesGcmChannel<FramedTransport*>>(
            &m_framed_client, client_config);
          m_client->get_writer().write(SharedBuffer());
        } catch(...) {
          client_error = std::current_exception();
        }
        server.join();
        return std::pair(client_error, server_error);
      }

      void connect() {
        auto& keys = get_keys();
        auto [client, server] = connect(
          AesGcmClientConfig(keys.m_public_key),
          AesGcmServerConfig(keys.m_private_key));
        REQUIRE(!client);
        REQUIRE(!server);
        reset(m_client_transport.m_writer.m_output);
        reset(m_server_transport.m_writer.m_output);
      }
  };
}

TEST_SUITE("AesGcmChannel") {
  TEST_CASE("pending_handshake") {
    auto session = Session();
    auto& keys = get_keys();
    session.m_client =
      std::make_unique<AesGcmChannel<Session::FramedTransport*>>(
        &session.m_framed_client, AesGcmClientConfig(keys.m_public_key));
    REQUIRE(!session.m_client->get_reader().poll());
    auto entered = std::latch(2);
    auto received = SharedBuffer();
    auto read_error = std::exception_ptr();
    auto write_error = std::exception_ptr();
    auto reading = std::jthread([&] {
      entered.count_down();
      auto result = try_call([&] {
        session.m_client->get_reader().read(out(received));
      });
      read_error = result.get_exception();
    });
    auto writing = std::jthread([&] {
      entered.count_down();
      auto result = try_call([&] {
        session.m_client->get_writer().write(from<SharedBuffer>("request"));
      });
      write_error = result.get_exception();
    });
    entered.wait();
    SUBCASE("success") {
      session.m_server =
        std::make_unique<AesGcmChannel<Session::FramedTransport*>>(
          &session.m_framed_server, AesGcmServerConfig(keys.m_private_key));
      auto request = SharedBuffer();
      session.m_server->get_reader().read(out(request));
      REQUIRE(request == "request");
      session.m_server->get_writer().write(from<SharedBuffer>("reply"));
      reading.join();
      writing.join();
      REQUIRE(!read_error);
      REQUIRE(!write_error);
      REQUIRE(received == "reply");
    }
    SUBCASE("close") {
      session.m_client->get_connection().close();
      reading.join();
      writing.join();
      REQUIRE(read_error);
      REQUIRE(write_error);
      REQUIRE_THROWS_AS(std::rethrow_exception(read_error), EndOfFileException);
      REQUIRE_THROWS_AS(
        std::rethrow_exception(write_error), EndOfFileException);
      REQUIRE(is_empty(received));
    }
  }

  TEST_CASE("destroy_pending_handshake") {
    auto session = Session();
    auto& keys = get_keys();
    auto client = std::make_unique<AesGcmChannel<Session::FramedTransport*>>(
      &session.m_framed_client, AesGcmClientConfig(keys.m_public_key));
    client.reset();
    REQUIRE(session.m_client_transport.m_connection.m_is_closed);
  }

  TEST_CASE("server_context") {
    auto& keys = get_keys();
    auto context = make_server_context(AesGcmServerConfig(keys.m_private_key));
    REQUIRE(context);
    REQUIRE(SSL_CTX_get0_privatekey(context.get()));
    for(auto key : {std::string(), std::string("invalid"), keys.m_public_key}) {
      REQUIRE_THROWS_AS(
        make_server_context(AesGcmServerConfig(key)), ConnectException);
    }
  }

  TEST_CASE("record_transport") {
    auto& keys = get_keys();
    auto session = Session();
    auto client_transport =
      BasicChannel(&session.m_client_transport.m_identifier,
        &session.m_client_transport.m_connection,
        &session.m_client_input, &session.m_client_transport.m_writer);
    auto server_transport =
      BasicChannel(&session.m_server_transport.m_identifier,
        &session.m_server_transport.m_connection,
        &session.m_server_input, &session.m_server_transport.m_writer);
    auto server =
      std::unique_ptr<AesGcmChannel<decltype(server_transport)*>>();
    auto error = std::exception_ptr();
    auto thread = std::jthread([&] {
      try {
        server = std::make_unique<AesGcmChannel<decltype(server_transport)*>>(
          &server_transport, AesGcmServerConfig(keys.m_private_key));
        server->get_writer().write(SharedBuffer());
      } catch(...) {
        error = std::current_exception();
      }
    });
    auto client =
      AesGcmChannel(&client_transport, AesGcmClientConfig(keys.m_public_key));
    client.get_writer().write(SharedBuffer());
    thread.join();
    REQUIRE(!error);
    reset(session.m_client_transport.m_writer.m_output);
    reset(session.m_server_transport.m_writer.m_output);
    auto message = from<SharedBuffer>("payload");
    client.get_writer().write(message);
    client.get_writer().write(message);
    REQUIRE(session.m_client_transport.m_writer.m_output.get_size() ==
      2 * (message.get_size() + AesGcmEncoder::TAG_SIZE));
    for(auto i = 0; i != 2; ++i) {
      auto received = SharedBuffer();
      REQUIRE(server->get_reader().read(out(received)) == message.get_size());
      REQUIRE(received == message);
    }
    server->get_writer().write(message);
    REQUIRE(session.m_server_transport.m_writer.m_output.get_size() ==
      message.get_size() + AesGcmEncoder::TAG_SIZE);
    auto received = SharedBuffer();
    REQUIRE(client.get_reader().read(out(received)) == message.get_size());
    REQUIRE(received == message);
  }

  TEST_CASE("communication") {
    auto session = Session();
    session.connect();
    REQUIRE(&session.m_client->get_identifier() ==
      &session.m_client_transport.get_identifier());
    REQUIRE(&session.m_client->get_connection() ==
      &session.m_client_transport.get_connection());
    auto client = Channel(session.m_client.get());
    auto server = Channel(session.m_server.get());
    auto message = from<SharedBuffer>("confidential application payload");
    client.get_writer().write(message);
    server.get_writer().write(message);
    auto received = SharedBuffer();
    REQUIRE(server.get_reader().read(out(received), 5) == 5);
    REQUIRE(server.get_reader().poll());
    REQUIRE(server.get_reader().read(out(received)) == message.get_size() - 5);
    REQUIRE(received == message);
    reset(received);
    REQUIRE(client.get_reader().read(out(received)) == message.get_size());
    REQUIRE(received == message);
    auto& outgoing = session.m_client_transport.m_writer.m_output;
    REQUIRE(std::string_view(outgoing.get_data(), outgoing.get_size()).find(
      "confidential application payload") == std::string_view::npos);
    REQUIRE(outgoing != session.m_server_transport.m_writer.m_output);
    client.get_connection().close();
    REQUIRE(session.m_client_transport.m_connection.m_is_closed);
    REQUIRE_THROWS_AS(client.get_writer().write(message), IOException);
    REQUIRE_THROWS_AS(server.get_reader().read(out(received)), IOException);
  }

  TEST_CASE("record_sizes") {
    auto session = Session();
    session.connect();
    session.m_client->get_writer().write(SharedBuffer());
    REQUIRE(session.m_client_transport.m_writer.m_output.get_size() == 0);
    for(auto size : {AesGcmEncoder::MAXIMUM_SIZE,
        AesGcmEncoder::MAXIMUM_SIZE + 1, std::size_t(70000)}) {
      auto message = from<SharedBuffer>(std::string(size, 'x'));
      session.m_client->get_writer().write(message);
      auto received = SharedBuffer();
      read_exact(
        session.m_server->get_reader(), out(received), message.get_size());
      REQUIRE(received == message);
    }
  }

  TEST_CASE("session_keys") {
    auto first = Session();
    auto second = Session();
    first.connect();
    second.connect();
    auto message = from<SharedBuffer>("same message");
    first.m_client->get_writer().write(message);
    second.m_client->get_writer().write(message);
    REQUIRE(first.m_client_transport.m_writer.m_output !=
      second.m_client_transport.m_writer.m_output);
  }

  TEST_CASE("authentication") {
    auto& keys = get_keys();
    auto client_config = AesGcmClientConfig(keys.m_public_key);
    auto server_config = AesGcmServerConfig(keys.m_private_key);
    auto session = Session();
    SUBCASE("missing_public_key") {
      client_config.m_public_key.clear();
    }
    SUBCASE("invalid_public_key") {
      client_config.m_public_key = "invalid";
    }
    SUBCASE("missing_private_key") {
      server_config.m_private_key.clear();
    }
    SUBCASE("invalid_private_key") {
      server_config.m_private_key = "invalid";
    }
    SUBCASE("mismatched_private_key") {
      server_config.m_private_key = make_keys("EC").m_private_key;
    }
    SUBCASE("public_key") {
      client_config.m_public_key = make_keys("EC").m_public_key;
    }
    SUBCASE("public_key_as_private_key") {
      server_config.m_private_key = keys.m_public_key;
    }
    SUBCASE("tampered_handshake") {
      session.m_server_transport.m_writer.m_is_tampered = true;
    }
    auto [client, server] = session.connect(client_config, server_config);
    REQUIRE(client);
    REQUIRE(server);
    REQUIRE(session.m_client_transport.m_connection.m_is_closed);
    REQUIRE(session.m_server_transport.m_connection.m_is_closed);
    REQUIRE_THROWS_AS(std::rethrow_exception(client), IOException);
    REQUIRE_THROWS_AS(std::rethrow_exception(server), IOException);
  }

  TEST_CASE("key_types") {
    auto algorithm = std::string_view("EC");
    SUBCASE("ec") {}
    SUBCASE("rsa") {
      algorithm = "RSA";
    }
    SUBCASE("ed25519") {
      algorithm = "ED25519";
    }
    auto keys = make_keys(algorithm);
    auto session = Session();
    auto [client, server] =
      session.connect(AesGcmClientConfig(keys.m_public_key),
        AesGcmServerConfig(keys.m_private_key));
    REQUIRE(!client);
    REQUIRE(!server);
    session.m_client->get_writer().write(from<SharedBuffer>("payload"));
    auto message = SharedBuffer();
    session.m_server->get_reader().read(out(message));
    REQUIRE(message == "payload");
  }

  TEST_CASE("read_failure") {
    auto session = Session();
    session.connect();
    auto message = from<SharedBuffer>("payload");
    auto output = from<SharedBuffer>("unchanged");
    SUBCASE("authentication_tag") {
      session.m_client_transport.m_writer.m_is_tampered = true;
      session.m_client->get_writer().write(message);
    }
    SUBCASE("oversized_record") {
      auto record = from<SharedBuffer>(std::string(
        AesGcmEncoder::MAXIMUM_SIZE + AesGcmEncoder::TAG_SIZE + 1, '\0'));
      session.m_framed_client.get_writer().write(record);
    }
    REQUIRE_THROWS_AS(
      session.m_server->get_reader().read(out(output)), IOException);
    REQUIRE(output == "unchanged");
    REQUIRE(!session.m_client_transport.m_connection.m_is_closed);
    REQUIRE(!session.m_server_transport.m_connection.m_is_closed);
    REQUIRE_THROWS_AS(
      session.m_server->get_reader().read(out(output)), IOException);
    REQUIRE(output == "unchanged");
    session.m_server->get_writer().write(message);
    auto received = SharedBuffer();
    session.m_client->get_reader().read(out(received));
    REQUIRE(received == message);
    session.m_server->get_connection().close();
    REQUIRE(session.m_server_transport.m_connection.m_is_closed);
  }

  TEST_CASE("write_failure") {
    auto session = Session();
    session.connect();
    auto message = from<SharedBuffer>("payload");
    session.m_client_transport.m_writer.m_is_failed = true;
    REQUIRE_THROWS_AS(
      session.m_client->get_writer().write(message), IOException);
    REQUIRE(!session.m_client_transport.m_connection.m_is_closed);
    REQUIRE(!session.m_server_transport.m_connection.m_is_closed);
    session.m_server->get_writer().write(message);
    auto received = SharedBuffer();
    session.m_client->get_reader().read(out(received));
    REQUIRE(received == message);
    session.m_client->get_connection().close();
    REQUIRE(session.m_client_transport.m_connection.m_is_closed);
  }

  TEST_CASE("externally_synchronized_writes") {
    auto session = Session();
    session.connect();
    auto mutex = std::mutex();
    auto writers = std::vector<std::jthread>();
    for(auto i = 0; i != 4; ++i) {
      writers.emplace_back([&] {
        for(auto j = 0; j != 25; ++j) {
          auto lock = std::lock_guard(mutex);
          session.m_client->get_writer().write(from<SharedBuffer>("message"));
        }
      });
    }
    for(auto i = 0; i != 100; ++i) {
      auto message = SharedBuffer();
      session.m_server->get_reader().read(out(message));
      REQUIRE(message == "message");
    }
  }

  TEST_CASE("invalid_handshake") {
    auto& keys = get_keys();
    auto session = Session();
    auto message = from<SharedBuffer>("invalid handshake contents");
    session.m_framed_client.get_writer().write(message);
    auto server = AesGcmChannel(&session.m_framed_server,
      AesGcmServerConfig(keys.m_private_key));
    REQUIRE_THROWS_AS(
      server.get_writer().write(SharedBuffer()), ConnectException);
    REQUIRE(session.m_server_transport.m_connection.m_is_closed);
  }
}
