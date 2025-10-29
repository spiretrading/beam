#include <doctest/doctest.h>
#include "Beam/ServicesTests/ServiceClientFixture.hpp"
#include "Beam/UidService/ServiceUidClient.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

namespace {
  struct Fixture : ServiceClientFixture {
    using TestUidClient = ServiceUidClient<TestServiceProtocolClientBuilder>;
    std::unique_ptr<TestUidClient> m_client;

    Fixture() {
      register_uid_services(out(m_server.get_slots()));
      m_client = make_client<TestUidClient>();
    }
  };
}

TEST_SUITE("ServiceUidClient") {
  TEST_CASE("single_uid_request") {
    auto fixture = Fixture();
    auto received_request = false;
    auto initial_uid = std::uint64_t(123);
    fixture.on_request<ReserveUidsService>(
      [&] (auto& request, auto block_size) {
        REQUIRE(block_size > 0);
        received_request = true;
        request.set(initial_uid);
      });
    auto result = fixture.m_client->load_next_uid();
    REQUIRE(received_request);
    REQUIRE(result == initial_uid);
  }

  TEST_CASE("sequential_uid_requests") {
    auto fixture = Fixture();
    auto request_count = 0;
    auto initial_uid = std::uint64_t(123);
    fixture.on_request<ReserveUidsService>(
      [&] (auto& request, auto block_size) {
        REQUIRE(block_size > 0);
        ++request_count;
        request.set(initial_uid);
      });
    auto uid_result_a = fixture.m_client->load_next_uid();
    REQUIRE(request_count == 1);
    REQUIRE(uid_result_a == initial_uid);
    auto uid_result_b = fixture.m_client->load_next_uid();
    REQUIRE(request_count == 1);
    REQUIRE(uid_result_b == initial_uid + 1);
  }

  TEST_CASE("simultaneous_uid_requests") {
    auto fixture = Fixture();
    auto request_count = 0;
    auto initial_uid = std::uint64_t(123);
    fixture.on_request<ReserveUidsService>(
      [&] (auto& request, auto block_size) {
        REQUIRE(block_size > 0);
        ++request_count;
        request.set(initial_uid);
      });
    auto uid_result_a = fixture.m_client->load_next_uid();
    REQUIRE(uid_result_a == initial_uid);
    auto uid_result_b = fixture.m_client->load_next_uid();
    REQUIRE(uid_result_b == initial_uid + 1);
    REQUIRE(request_count == 1);
  }

  TEST_CASE("multiple_server_requests") {
    auto fixture = Fixture();
    auto request_count = 0;
    auto initial_uid = std::uint64_t(123);
    auto request_block_size = std::uint64_t();
    fixture.on_request<ReserveUidsService>(
      [&] (auto& request, auto block_size) {
        REQUIRE(block_size > 0);
        request_block_size = block_size;
        ++request_count;
        if(request_count == 1) {
          request.set(initial_uid);
        } else {
          request.set(1000 * initial_uid);
        }
      });
    auto uid_result_a = fixture.m_client->load_next_uid();
    auto initial_block_results = std::vector<std::uint64_t>();
    for(auto i = std::uint64_t(1); i < request_block_size; ++i) {
      auto uid_result = fixture.m_client->load_next_uid();
      initial_block_results.push_back(uid_result);
    }
    auto secondary_block_results = std::vector<std::uint64_t>();
    for(auto i = std::uint64_t(0); i < request_block_size; ++i) {
      auto uid_result = fixture.m_client->load_next_uid();
      secondary_block_results.push_back(uid_result);
    }
    REQUIRE(request_count == 2);
    REQUIRE(uid_result_a == initial_uid);
    auto counter = std::uint64_t(1);
    for(auto uid : initial_block_results) {
      REQUIRE(uid == initial_uid + counter);
      ++counter;
    }
    counter = 0;
    for(auto uid : secondary_block_results) {
      REQUIRE(uid == 1000 * initial_uid + counter);
      ++counter;
    }
  }
}
