#include <doctest/doctest.h>
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
  TEST_CASE("receive_on_demand") {
    auto fixture = Fixture();
    REQUIRE_FALSE(fixture.m_receiver->poll());
    REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
    fixture.send("first");
    REQUIRE(fixture.m_receiver->poll());
    REQUIRE(fixture.m_context.poll() == 0);
    auto packet = DatagramPacket<SharedBuffer>();
    auto reader = RoutineHandler(spawn([&] {
      fixture.m_receiver->receive(out(packet));
    }));
    flush_pending_routines();
    fixture.m_context.restart();
    fixture.m_context.run_one();
    reader.wait();
    REQUIRE(packet.get_data() == "first");
    REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
    fixture.send("second");
    REQUIRE(fixture.m_context.poll() == 0);
    REQUIRE(fixture.m_socket->m_socket.available() == 6);
  }

  TEST_CASE("datagrams") {
    auto fixture = Fixture();
    auto data = from<SharedBuffer>("prefix");
    auto address = IpAddress();
    auto size = std::size_t(5);
    auto message = std::string_view("first");
    SUBCASE("append") {}
    SUBCASE("bounded") {
      size = 2;
    }
    SUBCASE("empty") {
      message = "";
    }
    fixture.send(message);
    auto result = Queue<std::size_t>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        result.push(fixture.m_receiver->receive(out(data), size, out(address)));
      } catch(const std::exception&) {
        result.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    fixture.m_context.run_one();
    reader.wait();
    REQUIRE(result.pop() == std::min(size, message.size()));
    REQUIRE(data == "prefix" + std::string(message.substr(0, size)));
    REQUIRE(address == IpAddress("127.0.0.1",
      fixture.m_sender.local_endpoint().port()));
    REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
  }

  TEST_CASE("receive_after_timeout") {
    auto options = UdpSocketOptions();
    options.m_timeout = boost::posix_time::milliseconds(50);
    auto fixture = Fixture(options);
    auto data = from<SharedBuffer>("prefix");
    for(auto i = 0; i != 2; ++i) {
      auto results = Queue<std::size_t>();
      auto reader = RoutineHandler(spawn([&] {
        try {
          results.push(fixture.m_receiver->receive(out(data), 1024));
        } catch(const std::exception&) {
          results.close(std::current_exception());
        }
      }));
      flush_pending_routines();
      fixture.m_context.restart();
      fixture.m_context.run();
      reader.wait();
      REQUIRE_THROWS_AS(results.pop(), EndOfFileException);
      REQUIRE(data == "prefix");
      REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
      REQUIRE(fixture.m_socket->m_socket.is_open());
    }
    fixture.send("next");
    auto results = Queue<std::size_t>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        results.push(fixture.m_receiver->receive(out(data), 1024));
      } catch(const std::exception&) {
        results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    fixture.m_context.restart();
    fixture.m_context.run();
    reader.wait();
    REQUIRE(results.pop() == 4);
    REQUIRE(data == "prefixnext");
    REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
    REQUIRE(fixture.m_socket->m_socket.is_open());
  }

  TEST_CASE("receive_interruption") {
    auto options = UdpSocketOptions();
    SUBCASE("close") {}
    SUBCASE("timeout") {
      options.m_timeout = boost::posix_time::seconds(0);
    }
    auto fixture = Fixture(options);
    auto results = Queue<std::size_t>();
    auto reader = RoutineHandler(spawn([&] {
      try {
        auto packet = DatagramPacket<SharedBuffer>();
        results.push(fixture.m_receiver->receive(out(packet)));
      } catch(const std::exception&) {
        results.close(std::current_exception());
      }
    }));
    flush_pending_routines();
    auto closer = RoutineHandler();
    if(options.m_timeout == boost::posix_time::pos_infin) {
      closer = spawn([&] {
        fixture.m_socket->close();
      });
      flush_pending_routines();
    }
    fixture.m_context.run();
    reader.wait();
    closer.wait();
    REQUIRE_THROWS_AS(results.pop(), EndOfFileException);
    REQUIRE_FALSE(fixture.m_socket->m_is_read_pending);
  }
}
