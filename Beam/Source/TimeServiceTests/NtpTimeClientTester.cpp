module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

module Beam;

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using TestChannel =
    WrapperChannel<NullChannel, PipedReader, NullConnection, PipedWriter>;
  using TestTimeClient = NtpTimeClient<TestChannel, TriggerTimer*>;

  auto make_ntp_response(ptime receive_time, ptime transmit_time) {
    auto packet = NtpPacket();
    packet.fill(0);
    auto receive_timestamp = ntp_time_from_posix_time(receive_time);
    std::memcpy(packet.data() + RECEIVE_TIMESTAMP_OFFSET, &receive_timestamp,
      sizeof(receive_timestamp));
    auto transmit_timestamp = ntp_time_from_posix_time(transmit_time);
    std::memcpy(packet.data() + TRANSMIT_TIMESTAMP_OFFSET, &transmit_timestamp,
      sizeof(transmit_timestamp));
    return packet;
  }

  auto make_ntp_response(const SharedBuffer& request, ptime receive_time,
      ptime transmit_time) {
    auto packet = make_ntp_response(receive_time, transmit_time);
    std::memcpy(packet.data() + ORIGIN_TIMESTAMP_OFFSET,
      request.get_data() + TRANSMIT_TIMESTAMP_OFFSET, sizeof(std::uint64_t));
    return packet;
  }

  auto to_buffer(const NtpPacket& packet) {
    auto buffer = SharedBuffer();
    append(buffer, packet.data(), packet.size());
    return buffer;
  }

  auto is_close(ptime time, ptime expected) {
    return std::abs((time - expected).total_milliseconds()) < 500;
  }

  struct TestNtpServer {
    PipedReader m_requests;
    std::unique_ptr<TestChannel> m_channel;
    PipedWriter m_responses;
    ptime m_receive_time;
    ptime m_transmit_time;
    SharedBuffer m_request;
    bool m_is_serving;
    RoutineHandler m_routine;

    TestNtpServer()
      : m_channel(std::make_unique<TestChannel>(
          init(), init(), init(), init(Ref(m_requests)))),
        m_responses(Ref(m_channel->get_reader())),
        m_is_serving(false) {}

    void serve(ptime time) {
      serve(time, time);
    }

    void serve(ptime receive_time, ptime transmit_time) {
      m_receive_time = receive_time;
      m_transmit_time = transmit_time;
      if(m_is_serving) {
        return;
      }
      m_is_serving = true;
      m_routine = spawn([this] {
        try {
          while(true) {
            auto request = SharedBuffer();
            m_requests.read(out(request));
            m_request = request;
            m_responses.write(to_buffer(
              make_ntp_response(request, m_receive_time, m_transmit_time)));
          }
        } catch(const std::exception&) {}
      });
    }
  };

  void stage_response(TestNtpServer& server, ptime time) {
    server.m_responses.write(to_buffer(make_ntp_response(time, time)));
  }
}

TEST_SUITE("NtpTimeClient") {
  TEST_CASE("posix_time_from_ntp_time") {
    auto encoding = std::array<std::uint8_t, 8>();
    encoding.fill(0);
    encoding[3] = 1;
    encoding[4] = 0x80;
    auto time = posix_time_from_ntp_time(encoding.data());
    REQUIRE(time == time_from_string("1900-01-01 00:00:01.500"));
    encoding.fill(0);
    REQUIRE(posix_time_from_ntp_time(encoding.data()).is_not_a_date_time());
  }

  TEST_CASE("ntp_time_round_trip") {
    auto time = time_from_string("2026-07-06 09:30:00.500");
    auto encoding = ntp_time_from_posix_time(time);
    auto decoding = posix_time_from_ntp_time(
      reinterpret_cast<const std::uint8_t*>(&encoding));
    REQUIRE(decoding == time);
  }

  TEST_CASE("synchronize_on_construction") {
    auto timer = TriggerTimer();
    auto server = TestNtpServer();
    auto time = time_from_string("2026-07-06 10:00:00");
    server.serve(time);
    auto channels = std::vector<std::unique_ptr<TestChannel>>();
    channels.push_back(std::move(server.m_channel));
    auto client = TestTimeClient(std::move(channels), &timer);
    REQUIRE(is_close(client.get_time(), time));
    REQUIRE(server.m_request.get_size() == NTP_PACKET_SIZE);
    REQUIRE(server.m_request.get_data()[0] == 0x1B);
    auto transmission = posix_time_from_ntp_time(
      reinterpret_cast<const std::uint8_t*>(server.m_request.get_data()) +
        TRANSMIT_TIMESTAMP_OFFSET);
    REQUIRE_FALSE(transmission.is_not_a_date_time());
    client.close();
    REQUIRE_THROWS(client.get_time());
  }

  TEST_CASE("average_offsets_across_sources") {
    auto timer = TriggerTimer();
    auto servers = std::array<TestNtpServer, 2>();
    servers[0].serve(time_from_string("2026-07-06 10:00:00"));
    servers[1].serve(time_from_string("2026-07-06 10:10:00"));
    auto channels = std::vector<std::unique_ptr<TestChannel>>();
    channels.push_back(std::move(servers[0].m_channel));
    channels.push_back(std::move(servers[1].m_channel));
    auto client = TestTimeClient(std::move(channels), &timer);
    REQUIRE(is_close(
      client.get_time(), time_from_string("2026-07-06 10:05:00")));
  }

  TEST_CASE("skip_unresponsive_sources") {
    auto timer = TriggerTimer();
    auto servers = std::array<TestNtpServer, 2>();
    servers[0].m_responses.close();
    servers[1].serve(time_from_string("2026-07-06 10:00:00"));
    auto channels = std::vector<std::unique_ptr<TestChannel>>();
    channels.push_back(std::move(servers[0].m_channel));
    channels.push_back(std::move(servers[1].m_channel));
    auto client = TestTimeClient(std::move(channels), &timer);
    REQUIRE(is_close(
      client.get_time(), time_from_string("2026-07-06 10:00:00")));
  }

  TEST_CASE("reject_invalid_sources") {
    auto require_rejection = [] (TestNtpServer& server) {
      auto timer = TriggerTimer();
      auto channels = std::vector<std::unique_ptr<TestChannel>>();
      channels.push_back(std::move(server.m_channel));
      REQUIRE_THROWS_AS(
        (void)TestTimeClient(std::move(channels), &timer), ConnectException);
    };
    auto failure = TestNtpServer();
    failure.m_responses.close();
    require_rejection(failure);
    auto blank = TestNtpServer();
    auto packet = NtpPacket();
    packet.fill(0);
    blank.m_responses.write(to_buffer(packet));
    blank.m_responses.close();
    require_rejection(blank);
    auto runt = TestNtpServer();
    auto fragment = SharedBuffer();
    append(fragment, packet.data(), 8);
    runt.m_responses.write(fragment);
    require_rejection(runt);
    auto time = time_from_string("2026-07-06 10:00:00");
    auto backdated = TestNtpServer();
    backdated.serve(time, time + seconds(2));
    require_rejection(backdated);
    auto delayed = TestNtpServer();
    delayed.serve(time, time - seconds(2));
    require_rejection(delayed);
  }

  TEST_CASE("resynchronize_on_timer_expiry") {
    auto timer = TriggerTimer();
    auto server = TestNtpServer();
    server.serve(time_from_string("2026-07-06 10:00:00"));
    auto channels = std::vector<std::unique_ptr<TestChannel>>();
    channels.push_back(std::move(server.m_channel));
    auto client = TestTimeClient(std::move(channels), &timer);
    server.serve(time_from_string("2026-07-06 11:00:00"));
    timer.trigger();
    flush_pending_routines();
    REQUIRE(is_close(
      client.get_time(), time_from_string("2026-07-06 11:00:00")));
  }

  TEST_CASE("discard_stale_responses") {
    auto timer = TriggerTimer();
    auto servers = std::array<TestNtpServer, 4>();
    auto time = time_from_string("2026-07-06 12:00:00");
    stage_response(servers[0], time - seconds(1800));
    for(auto& server : servers) {
      server.serve(time);
    }
    auto channels = std::vector<std::unique_ptr<TestChannel>>();
    for(auto& server : servers) {
      channels.push_back(std::move(server.m_channel));
    }
    auto client = TestTimeClient(std::move(channels), &timer);
    REQUIRE(is_close(client.get_time(), time));
  }
}
