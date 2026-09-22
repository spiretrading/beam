#include <atomic>
#include <doctest/doctest.h>
#include "Beam/IO/StaticBuffer.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;

namespace {
  struct Fixture {
    boost::asio::io_context m_context;
    std::shared_ptr<Details::UdpSocketEntry> m_socket;
    boost::asio::ip::udp::socket m_sender;
    std::unique_ptr<UdpSocketReceiver> m_receiver;

    Fixture()
      : Fixture(UdpSocketOptions()) {}

    explicit Fixture(const UdpSocketOptions& options)
        : m_socket(std::make_shared<Details::UdpSocketEntry>(
            m_context, m_context, boost::asio::ip::udp::v4())),
          m_sender(m_context, boost::asio::ip::udp::endpoint(
            boost::asio::ip::address_v4::loopback(), 0)) {
      m_socket->m_socket.bind(boost::asio::ip::udp::endpoint(
        boost::asio::ip::address_v4::loopback(), 0));
      m_socket->m_is_open = true;
      m_receiver = std::make_unique<UdpSocketReceiver>(options, m_socket);
    }

    ~Fixture() {
      {
        auto lock = std::lock_guard(m_socket->m_mutex);
        m_socket->m_is_open = false;
        m_socket->m_socket.close();
      }
      m_context.restart();
      m_context.run();
    }

    void send(std::string_view message) {
      m_sender.send_to(boost::asio::buffer(message.data(), message.size()),
        m_socket->m_socket.local_endpoint());
    }
  };
}

TEST_SUITE("UdpSocketReceiver") {
  TEST_CASE("buffered_datagrams") {
    auto fixture = Fixture();
    auto packet = DatagramPacket<SharedBuffer>();
    auto reader = RoutineHandler(spawn([&] {
      fixture.m_receiver->receive(out(packet));
    }));
    flush_pending_routines();
    fixture.send("first");
    REQUIRE(fixture.m_context.run_one() == 1);
    reader.wait();
    REQUIRE(packet.get_data() == "first");
    fixture.m_context.restart();
    fixture.send("second");
    REQUIRE(fixture.m_context.run_one() == 1);
    REQUIRE(fixture.m_socket->m_socket.available() == 0);
    reset(packet.get_data());
    REQUIRE(fixture.m_receiver->receive(out(packet)) == 6);
    REQUIRE(packet.get_data() == "second");
    REQUIRE(packet.get_address() == IpAddress("127.0.0.1",
      fixture.m_sender.local_endpoint().port()));
  }

  TEST_CASE("datagrams") {
    auto fixture = Fixture();
    REQUIRE_FALSE(fixture.m_receiver->poll());
    for(auto message : {"one", "", "three"}) {
      fixture.send(message);
      REQUIRE(fixture.m_context.run_one() == 1);
    }
    auto packet = DatagramPacket<SharedBuffer>();
    SUBCASE("order") {
      for(auto message : {"one", "", "three"}) {
        REQUIRE(fixture.m_receiver->poll());
        reset(packet.get_data());
        REQUIRE(fixture.m_receiver->receive(out(packet)) ==
          std::string_view(message).size());
        REQUIRE(packet.get_data() == message);
        REQUIRE(packet.get_address() == IpAddress("127.0.0.1",
          fixture.m_sender.local_endpoint().port()));
      }
      REQUIRE_FALSE(fixture.m_receiver->poll());
    }
    SUBCASE("append") {
      packet.get_data() = from<SharedBuffer>("prefix");
      REQUIRE(fixture.m_receiver->receive(out(packet)) == 3);
      REQUIRE(packet.get_data() == "prefixone");
    }
    SUBCASE("bounded_read") {
      REQUIRE(fixture.m_receiver->receive(out(packet), 2) == 2);
      REQUIRE(packet.get_data() == "on");
      reset(packet.get_data());
      REQUIRE(fixture.m_receiver->receive(out(packet)) == 0);
    }
    SUBCASE("fixed_buffer") {
      auto fixed = DatagramPacket<StaticBuffer<2>>();
      REQUIRE(fixture.m_receiver->receive(out(fixed)) == 2);
      REQUIRE(fixed.get_data() == "on");
      REQUIRE(fixture.m_receiver->receive(out(packet)) == 0);
    }
  }

  TEST_CASE("close") {
    auto fixture = Fixture();
    auto results = Queue<DatagramPacket<SharedBuffer>>();
    auto reader = RoutineHandler();
    SUBCASE("unstarted") {}
    SUBCASE("pending_read") {
      reader = spawn([&] {
        try {
          auto packet = DatagramPacket<SharedBuffer>();
          fixture.m_receiver->receive(out(packet));
          results.push(packet);
        } catch(const std::exception&) {
          results.close(std::current_exception());
        }
      });
      flush_pending_routines();
    }
    SUBCASE("buffered") {
      fixture.m_receiver->poll();
      fixture.send("queued");
      REQUIRE(fixture.m_context.run_one() == 1);
    }
    auto closer = RoutineHandler(spawn([&] {
      fixture.m_socket->close();
    }));
    flush_pending_routines();
    fixture.m_context.poll();
    closer.wait();
    reader.wait();
    REQUIRE_FALSE(fixture.m_receiver->poll());
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE_THROWS_AS(
      fixture.m_receiver->receive(out(packet)), EndOfFileException);
    if(results.is_broken()) {
      REQUIRE_THROWS_AS(results.pop(), EndOfFileException);
    }
  }

  TEST_CASE("deadline") {
    auto options = UdpSocketOptions();
    options.m_timeout = boost::posix_time::seconds(0);
    auto fixture = Fixture(options);
    auto results = Queue<DatagramPacket<SharedBuffer>>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        auto packet = DatagramPacket<SharedBuffer>();
        fixture.m_receiver->receive(out(packet));
        results.push(packet);
      } catch(const std::exception&) {
        results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(fixture.m_context.run_one() == 1);
    fixture.m_context.poll();
    reader.wait();
    REQUIRE_THROWS_AS(results.pop(), EndOfFileException);
    REQUIRE_FALSE(fixture.m_receiver->poll());
  }

  TEST_CASE("idle_receiver") {
    auto options = UdpSocketOptions();
    options.m_timeout = boost::posix_time::seconds(0);
    auto fixture = Fixture(options);
    REQUIRE_FALSE(fixture.m_receiver->poll());
    REQUIRE(fixture.m_context.poll() == 0);
    fixture.send("queued");
    REQUIRE(fixture.m_context.run_one() == 1);
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE(fixture.m_receiver->receive(out(packet)) == 6);
    REQUIRE(packet.get_data() == "queued");
    REQUIRE(fixture.m_context.poll() == 0);
    REQUIRE(fixture.m_socket->m_is_open);
  }

  TEST_CASE("receiver_destruction") {
    auto options = UdpSocketOptions();
    options.m_timeout = boost::posix_time::seconds(30);
    auto fixture = Fixture(options);
    auto packet = DatagramPacket<SharedBuffer>();
    auto reader = RoutineHandler(spawn([&] {
      fixture.m_receiver->receive(out(packet));
    }));
    flush_pending_routines();
    fixture.send("first");
    REQUIRE(fixture.m_context.run_one() == 1);
    reader.wait();
    auto closer = RoutineHandler(spawn([&] {
      fixture.m_receiver.reset();
    }));
    flush_pending_routines();
    fixture.m_context.run();
    closer.wait();
    REQUIRE_FALSE(fixture.m_socket->m_is_open);
    REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
  }

  TEST_CASE("deadline_close") {
    auto options = UdpSocketOptions();
    options.m_timeout = boost::posix_time::seconds(0);
    auto fixture = Fixture(options);
    auto results = Queue<DatagramPacket<SharedBuffer>>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        auto packet = DatagramPacket<SharedBuffer>();
        fixture.m_receiver->receive(out(packet));
        results.push(packet);
      } catch(const std::exception&) {
        results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    REQUIRE(fixture.m_context.run_one() == 1);
    auto is_closed = std::atomic_bool(false);
    auto closer = RoutineHandler(spawn([&] {
      fixture.m_socket->close();
      is_closed = true;
    }));
    flush_pending_routines();
    REQUIRE_FALSE(is_closed);
    fixture.m_context.poll();
    closer.wait();
    reader.wait();
    REQUIRE(is_closed);
    REQUIRE_THROWS_AS(results.pop(), EndOfFileException);
  }

  TEST_CASE("initial_poll") {
    auto fixture = Fixture();
    fixture.send("ready");
    REQUIRE(fixture.m_socket->m_socket.available() == 5);
    REQUIRE(fixture.m_receiver->poll());
    REQUIRE(fixture.m_context.run_one() == 1);
    REQUIRE(fixture.m_receiver->poll());
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE(fixture.m_receiver->receive(out(packet)) == 5);
    REQUIRE(packet.get_data() == "ready");
  }

  TEST_CASE("oversized_datagram") {
    auto options = UdpSocketOptions();
    options.m_max_datagram_size = 4;
    auto fixture = Fixture(options);
    fixture.m_receiver->poll();
    fixture.send("longer");
    REQUIRE(fixture.m_context.run_one() == 1);
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE(fixture.m_receiver->receive(out(packet)) == 4);
    REQUIRE(packet.get_data() == "long");
    reset(packet.get_data());
    auto reader = RoutineHandler(spawn([&] {
      fixture.m_receiver->receive(out(packet));
    }));
    flush_pending_routines();
    fixture.m_context.restart();
    fixture.send("next");
    REQUIRE(fixture.m_context.run_one() == 1);
    reader.wait();
    REQUIRE(packet.get_data() == "next");
  }

  TEST_CASE("receive_error") {
    auto fixture = Fixture();
    fixture.m_receiver->poll();
    fixture.m_socket->m_socket.cancel();
    REQUIRE(fixture.m_context.run_one() == 1);
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE_THROWS_AS(
      fixture.m_receiver->receive(out(packet)), EndOfFileException);
    auto reader = RoutineHandler(spawn([&] {
      fixture.m_receiver->receive(out(packet));
    }));
    flush_pending_routines();
    fixture.m_context.restart();
    fixture.send("next");
    REQUIRE(fixture.m_context.run_one() == 1);
    reader.wait();
    REQUIRE(packet.get_data() == "next");
  }

  TEST_CASE("senders") {
    auto fixture = Fixture();
    auto second_sender = boost::asio::ip::udp::socket(fixture.m_context,
      boost::asio::ip::udp::endpoint(
        boost::asio::ip::address_v4::loopback(), 0));
    fixture.m_receiver->poll();
    fixture.send("first");
    REQUIRE(fixture.m_context.run_one() == 1);
    auto second = std::string_view("second");
    second_sender.send_to(boost::asio::buffer(second.data(), second.size()),
      fixture.m_socket->m_socket.local_endpoint());
    REQUIRE(fixture.m_context.run_one() == 1);
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE(fixture.m_receiver->receive(out(packet)) == 5);
    REQUIRE(packet.get_data() == "first");
    REQUIRE(packet.get_address() == IpAddress("127.0.0.1",
      fixture.m_sender.local_endpoint().port()));
    reset(packet.get_data());
    REQUIRE(fixture.m_receiver->receive(out(packet)) == second.size());
    REQUIRE(packet.get_data() == second);
    REQUIRE(packet.get_address() ==
      IpAddress("127.0.0.1", second_sender.local_endpoint().port()));
  }
}
