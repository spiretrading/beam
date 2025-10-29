#include <future>
#include <memory>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"

using namespace Beam;

TEST_SUITE("LocalServerConnection") {
  TEST_CASE("accept_pending_channel") {
    auto server = LocalServerConnection();
    auto server_channel_future = std::async(std::launch::async, [&] {
      return server.accept();
    });
    auto client = std::make_unique<LocalClientChannel>("test", server);
    auto server_channel = server_channel_future.get();
    REQUIRE(server_channel);
    auto oss1 = std::stringstream();
    oss1 << server_channel->get_identifier();
    REQUIRE(oss1.str() == "test");
    auto oss2 = std::stringstream();
    oss2 << client->get_identifier();
    REQUIRE(oss2.str() == "client@test");
    auto message = from<SharedBuffer>("hello");
    server_channel->get_writer().write(message);
    auto read_buffer = SharedBuffer();
    client->get_reader().read(out(read_buffer));
    REQUIRE(read_buffer == message);
    auto reply = from<SharedBuffer>("ack");
    client->get_writer().write(reply);
    auto server_read = SharedBuffer();
    server_channel->get_reader().read(out(server_read));
    REQUIRE(server_read == reply);
  }

  TEST_CASE("accept_future_channel") {
    auto server = LocalServerConnection();
    auto client_future = std::async(std::launch::async, [&] {
      return std::make_unique<LocalClientChannel>("svc", server);
    });
    auto server_channel = server.accept();
    REQUIRE(server_channel);
    auto client = client_future.get();
    REQUIRE(client);
    auto oss_server = std::stringstream();
    oss_server << server_channel->get_identifier();
    REQUIRE(oss_server.str() == "svc");
    auto oss_client = std::stringstream();
    oss_client << client->get_identifier();
    REQUIRE(oss_client.str() == "client@svc");
  }

  TEST_CASE("multiple_clients") {
    auto server = LocalServerConnection();
    auto count = 3;
    auto client_futures =
      std::vector<std::future<std::unique_ptr<LocalClientChannel>>>();
    for(auto i = 0; i < count; ++i) {
      client_futures.push_back(std::async(std::launch::async, [&, i] {
        return std::make_unique<LocalClientChannel>(
          std::string("svc") + std::to_string(i), server);
      }));
    }
    auto server_channels = std::vector<std::unique_ptr<LocalServerChannel>>();
    for(auto i = 0; i < count; ++i) {
      server_channels.push_back(server.accept());
    }
    for(auto i = 0; i < count; ++i) {
      auto client = client_futures[i].get();
      REQUIRE(client);
      auto expected_name = std::string("svc") + std::to_string(i);
      auto oss_server = std::stringstream();
      oss_server << server_channels[i]->get_identifier();
      REQUIRE(oss_server.str() == expected_name);
      auto oss_client = std::stringstream();
      oss_client << client->get_identifier();
      REQUIRE(oss_client.str() == std::string("client@") + expected_name);
    }
  }

  TEST_CASE("client_connect_then_server_close") {
    auto server = LocalServerConnection();
    auto client_future = std::async(std::launch::async, [&] {
      return std::make_unique<LocalClientChannel>("x", server);
    });
    server.close();
    auto exception = [&] {
      try {
        client_future.get();
        return std::exception_ptr();
      } catch(...) {
        return std::current_exception();
      }
    }();
    REQUIRE(exception);
    REQUIRE_THROWS_AS(std::rethrow_exception(exception), ConnectException);
  }
}
