#include <doctest/doctest.h>
#include "Beam/IO/ChannelAdapterServerConnection.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/WrapperChannel.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

namespace {
  class TestServerConnection {
    public:
      using Channel = NullChannel;
      std::exception_ptr m_exception;
      bool m_is_closed;

      explicit TestServerConnection(std::exception_ptr exception) noexcept
        : m_exception(std::move(exception)),
          m_is_closed(false) {}

      std::unique_ptr<Channel> accept() {
        if(m_is_closed) {
          throw EndOfFileException();
        }
        if(m_exception) {
          std::rethrow_exception(m_exception);
        }
        return std::make_unique<Channel>(NamedChannelIdentifier("source"));
      }

      void close() {
        m_is_closed = true;
      }
  };

  using AdaptedChannel = WrapperChannel<std::unique_ptr<Channel>>;
  using Adapter =
    ChannelAdapterServerConnection<ServerConnection, AdaptedChannel>;
}

TEST_SUITE("ChannelAdapterServerConnection") {
  TEST_CASE("accept") {
    auto source = TestServerConnection(std::exception_ptr());
    auto adapted = static_cast<AdaptedChannel*>(nullptr);
    auto connection = Adapter(&source, [&] (std::unique_ptr<Channel> channel) {
      REQUIRE(channel);
      REQUIRE(to_string(channel->get_identifier()) == "source");
      auto result = std::make_unique<AdaptedChannel>(std::move(channel));
      adapted = result.get();
      return result;
    });
    auto channel = connection.accept();
    REQUIRE(channel);
    REQUIRE(channel.get() == adapted);
  }

  TEST_CASE("listener_failure") {
    auto failure = std::make_exception_ptr(std::logic_error("listener"));
    auto is_end_of_file = false;
    SUBCASE("end_of_file") {
      failure = std::make_exception_ptr(EndOfFileException());
      is_end_of_file = true;
    }
    SUBCASE("other_exception") {}
    auto source = TestServerConnection(failure);
    auto is_adapted = false;
    auto connection = Adapter(&source, [&] (std::unique_ptr<Channel> channel) {
      is_adapted = true;
      return std::make_unique<AdaptedChannel>(std::move(channel));
    });
    auto result = try_call([&] { connection.accept(); });
    REQUIRE(result.is_exception());
    if(is_end_of_file) {
      REQUIRE_THROWS_AS(result.get(), EndOfFileException);
    } else {
      REQUIRE_THROWS_AS(result.get(), std::logic_error);
    }
    REQUIRE(!is_adapted);
    connection.close();
    REQUIRE(source.m_is_closed);
    REQUIRE_THROWS_AS(connection.accept(), EndOfFileException);
    REQUIRE(!is_adapted);
  }

  TEST_CASE("adapter_failure") {
    auto failure = std::make_exception_ptr(std::logic_error("adapter"));
    auto is_end_of_file = false;
    SUBCASE("end_of_file") {
      failure = std::make_exception_ptr(EndOfFileException());
      is_end_of_file = true;
    }
    SUBCASE("other_exception") {}
    auto source = TestServerConnection(std::exception_ptr());
    auto connection = Adapter(&source, [&] (std::unique_ptr<Channel> channel) {
      if(failure) {
        std::rethrow_exception(failure);
      }
      return std::make_unique<AdaptedChannel>(std::move(channel));
    });
    auto result = try_call([&] { connection.accept(); });
    REQUIRE(result.is_exception());
    try {
      result.get();
    } catch(const std::exception& exception) {
      REQUIRE(dynamic_cast<const IOException*>(&exception));
      REQUIRE(!dynamic_cast<const EndOfFileException*>(&exception));
      auto nested = dynamic_cast<const std::nested_exception*>(&exception);
      REQUIRE(nested);
      REQUIRE(nested->nested_ptr());
      if(is_end_of_file) {
        REQUIRE_THROWS_AS(nested->rethrow_nested(), EndOfFileException);
      } else {
        REQUIRE_THROWS_AS(nested->rethrow_nested(), std::logic_error);
      }
    }
    REQUIRE(!source.m_is_closed);
    failure = nullptr;
    REQUIRE(connection.accept());
  }
}
