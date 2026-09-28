#include <memory>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Services/ApplicationDefinitions.hpp"

using namespace Beam;

namespace {
  struct Client {
    std::string m_username;

    Client(std::string username, Ref<int> attempts)
        : m_username(std::move(username)) {
      ++*attempts;
      if(*attempts == 1) {
        throw ConnectException();
      }
    }
  };

  struct Factory {
    std::unique_ptr<int> m_attempts;

    Factory()
      : m_attempts(std::make_unique<int>(0)) {}

    std::unique_ptr<int> operator ()() & {
      ++*m_attempts;
      if(*m_attempts == 1) {
        throw ConnectException();
      }
      return std::make_unique<int>(*m_attempts);
    }
  };
}

TEST_SUITE("ApplicationDefinitions") {
  TEST_CASE("connect") {
    SUBCASE("factory") {
      auto client = connect(Factory());
      REQUIRE(*client == 2);
    }
    SUBCASE("arguments") {
      auto attempts = 0;
      auto client = connect<Client>(std::string("user"), Ref(attempts));
      REQUIRE(client.m_username == "user");
      REQUIRE(attempts == 2);
    }
    SUBCASE("service_error") {
      auto attempts = 0;
      REQUIRE_THROWS_AS(connect([&] () -> int {
        ++attempts;
        throw ServiceRequestException("Invalid username or password.");
      }), ServiceRequestException);
      REQUIRE(attempts == 1);
    }
    SUBCASE("credentials") {
      auto attempts = 0;
      REQUIRE_THROWS_AS(connect([&] () -> int {
        ++attempts;
        if(attempts != 1) {
          throw std::runtime_error("Unexpected retry.");
        }
        try {
          throw ServiceRequestException("Invalid username or password.");
        } catch(const ServiceRequestException&) {
          std::throw_with_nested(ConnectException());
        }
      }), ServiceRequestException);
      REQUIRE(attempts == 1);
    }
    SUBCASE("connection_closed") {
      auto attempts = 0;
      auto client = connect([&] {
        ++attempts;
        if(attempts == 1) {
          try {
            throw ServiceRequestException("ServiceProtocolClient closed.");
          } catch(const ServiceRequestException&) {
            std::throw_with_nested(ConnectException());
          }
        }
        return attempts;
      });
      REQUIRE(client == 2);
      REQUIRE(attempts == 2);
    }
    SUBCASE("custom_wait") {
      auto attempts = 0;
      auto delays = std::vector<boost::posix_time::time_duration>();
      auto result = connect([&] {
        ++attempts;
        if(attempts <= 32) {
          throw ConnectException();
        }
        return attempts;
      }, [&] (boost::posix_time::time_duration&& delay) {
        delays.push_back(delay);
      });
      REQUIRE(result == 33);
      REQUIRE(delays.size() == 32);
      for(auto i = 0; i < 30; ++i) {
        REQUIRE(delays[i] == boost::posix_time::seconds(i + 1));
      }
      REQUIRE(delays[30] == boost::posix_time::seconds(30));
      REQUIRE(delays[31] == boost::posix_time::seconds(30));
    }
    SUBCASE("wait_interrupted") {
      auto attempts = 0;
      REQUIRE_THROWS_AS(connect([&] () -> int {
        ++attempts;
        throw ConnectException();
      }, [] (auto delay) {
        throw std::runtime_error("");
      }), std::runtime_error);
      REQUIRE(attempts == 1);
    }
  }
}
