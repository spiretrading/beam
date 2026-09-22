#include <doctest/doctest.h>
#include "Beam/Network/MulticastSocketChannel.hpp"
#include "Beam/Network/UdpSocketChannel.hpp"

using namespace Beam;

namespace {
  struct TestBuffer {
    std::string_view m_data;
    int& m_copies;

    TestBuffer(std::string_view data, int& copies)
      : m_data(data),
        m_copies(copies) {}

    TestBuffer(const TestBuffer& buffer)
        : m_data(buffer.m_data),
          m_copies(buffer.m_copies) {
      ++m_copies;
    }

    const char* get_data() const {
      return m_data.data();
    }

    std::size_t get_size() const {
      return m_data.size();
    }
  };
}

TEST_SUITE("UdpSocketSender") {
  TEST_CASE("writer_buffer") {
    auto context = boost::asio::io_context();
    auto receiver =
      boost::asio::ip::udp::socket(context, boost::asio::ip::udp::v4());
    receiver.set_option(boost::asio::socket_base::reuse_address(true));
    receiver.bind(
      boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
    auto interface = IpAddress("127.0.0.1", 0);
    auto copies = 0;
    auto buffer = TestBuffer("data", copies);
    SUBCASE("udp") {
      auto destination = IpAddress(
        "127.0.0.1", receiver.local_endpoint().port());
      auto channel = UdpSocketChannel(destination, interface);
      channel.get_writer().write(buffer);
    }
    SUBCASE("multicast") {
      auto destination = IpAddress(
        "239.255.0.7", receiver.local_endpoint().port());
      receiver.set_option(boost::asio::ip::multicast::join_group(
        boost::asio::ip::make_address_v4(destination.get_host()),
        boost::asio::ip::make_address_v4(interface.get_host())));
      auto options = MulticastSocketOptions();
      options.m_enable_loopback = true;
      auto channel = MulticastSocketChannel(destination, interface, options);
      channel.get_writer().write(buffer);
    }
    REQUIRE(copies == 0);
    auto received = SharedBuffer(buffer.get_size());
    auto sender = boost::asio::ip::udp::endpoint();
    REQUIRE(receiver.receive_from(boost::asio::buffer(
      received.get_mutable_data(), received.get_size()), sender) ==
      buffer.get_size());
    REQUIRE(received == "data");
  }

  TEST_CASE("send") {
    auto context = boost::asio::io_context();
    auto receiver = boost::asio::ip::udp::socket(context,
      boost::asio::ip::udp::endpoint(
        boost::asio::ip::address_v4::loopback(), 0));
    auto address = IpAddress("127.0.0.1", receiver.local_endpoint().port());
    auto socket = UdpSocket(address);
    for(auto message : {"one", "", "two"}) {
      auto buffer = from<SharedBuffer>(message);
      socket.get_sender().send(DatagramPacket(buffer, address));
      auto received = SharedBuffer(16);
      auto sender = boost::asio::ip::udp::endpoint();
      auto size = receiver.receive_from(boost::asio::buffer(
        received.get_mutable_data(), received.get_size()), sender);
      received.shrink(received.get_size() - size);
      REQUIRE(size == buffer.get_size());
      REQUIRE(received == buffer);
    }
  }
}
