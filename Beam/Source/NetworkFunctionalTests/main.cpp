#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <future>
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/MulticastSocketChannel.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Network/UdpSocketChannel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("UdpSocket") {
  TEST_CASE("basic_send_receive") {
    auto server_address = IpAddress("127.0.0.1", 15000);
    auto client_address = IpAddress("127.0.0.1", 15001);
    auto client_channel = UdpSocketChannel(server_address, client_address);
    auto server_channel = UdpSocketChannel(client_address, server_address);
    auto message = std::string("hello from client");
    auto send_buffer = SharedBuffer(message.data(), message.size());
    client_channel.get_writer().write(send_buffer);
    REQUIRE(server_channel.get_reader().poll());
    auto receive_buffer = SharedBuffer();
    REQUIRE(
      server_channel.get_reader().read(out(receive_buffer)) == message.size());
    REQUIRE(!server_channel.get_reader().poll());
    REQUIRE(receive_buffer == message);
  }

  TEST_CASE("large_datagram_handling") {
    auto server_address = IpAddress("127.0.0.1", 15002);
    auto client_address = IpAddress("127.0.0.1", 15003);
    auto options_large = UdpSocketOptions();
    options_large.m_max_datagram_size = 65507;
    options_large.m_receive_buffer_size = 65536;
    auto client_channel =
      UdpSocketChannel(server_address, client_address, options_large);
    auto server_channel =
      UdpSocketChannel(client_address, server_address, options_large);
    auto payload = std::string(options_large.m_max_datagram_size, 'x');
    auto send_buffer = from<SharedBuffer>(payload);
    client_channel.get_writer().write(send_buffer);
    auto receive_buffer = SharedBuffer();
    auto read_size = server_channel.get_reader().read(out(receive_buffer));
    REQUIRE(read_size == payload.size());
    REQUIRE(receive_buffer == payload);
  }

  TEST_CASE("max_datagram_size_respected") {
    auto server_address = IpAddress("127.0.0.1", 15006);
    auto client_address = IpAddress("127.0.0.1", 15007);
    auto options = UdpSocketOptions();
    options.m_max_datagram_size = 1024;
    options.m_receive_buffer_size = 4096;
    auto client_channel =
      UdpSocketChannel(server_address, client_address, options);
    auto server_channel =
      UdpSocketChannel(client_address, server_address, options);
    auto payload = std::string(2 * 1024, 'y');
    auto send_buffer = from<SharedBuffer>(payload);
    client_channel.get_writer().write(send_buffer);
    auto receive_buffer = SharedBuffer();
    auto read_size = server_channel.get_reader().read(out(receive_buffer));
    REQUIRE(read_size == options.m_max_datagram_size);
    REQUIRE(receive_buffer == payload.substr(0, options.m_max_datagram_size));
  }

  TEST_CASE("receive_buffer_size_enforced") {
    auto server_address = IpAddress("127.0.0.1", 15008);
    auto client_address = IpAddress("127.0.0.1", 15009);
    auto options = UdpSocketOptions();
    options.m_receive_buffer_size = 2048;
    options.m_max_datagram_size = 512;
    auto client_channel =
      UdpSocketChannel(server_address, client_address, options);
    auto server_channel =
      UdpSocketChannel(client_address, server_address, options);
    const auto MESSAGE_COUNT =
      options.m_receive_buffer_size / options.m_max_datagram_size;
    const auto SENT_COUNT = MESSAGE_COUNT + 1;
    for(auto i = 0; i < SENT_COUNT; ++i) {
      auto payload = std::string();
      auto sequence = std::to_string(i);
      payload += "sequence:";
      payload += sequence;
      if(payload.size() < options.m_max_datagram_size) {
        payload.append(options.m_max_datagram_size - payload.size(), 'A');
      }
      client_channel.get_writer().write(from<SharedBuffer>(payload));
    }
    auto received_indices = std::set<int>();
    while(server_channel.get_reader().poll()) {
      auto receive_buffer = SharedBuffer();
      auto read_size = server_channel.get_reader().read(out(receive_buffer));
      auto received = std::string(receive_buffer.get_data(), read_size);
      auto position = received.find("sequence:");
      if(position != std::string::npos) {
        try {
          auto num = std::stoi(received.substr(position + 4));
          received_indices.insert(num);
        } catch(...) {
        }
      }
    }
    REQUIRE(received_indices.size() == MESSAGE_COUNT);
  }

  TEST_CASE("timeout_behavior") {
    auto server_address = IpAddress("127.0.0.1", 15004);
    auto client_address = IpAddress("127.0.0.1", 15005);
    auto options_timeout = UdpSocketOptions();
    options_timeout.m_timeout = milliseconds(500);
    auto client_channel =
      UdpSocketChannel(server_address, client_address, options_timeout);
    auto server_channel =
      UdpSocketChannel(client_address, server_address, options_timeout);
    auto receive_buffer = SharedBuffer();
    REQUIRE_THROWS_AS(server_channel.get_reader().read(out(receive_buffer)),
      EndOfFileException);
  }
}

TEST_SUITE("TcpSocket") {
  TEST_CASE("basic_connect_send_receive") {
    auto server_address = IpAddress("127.0.0.1", 15020);
    auto server = TcpServerSocket(server_address);
    auto server_future = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      auto buffer = SharedBuffer();
      auto read_size = channel->get_reader().read(out(buffer));
      auto received = std::string(buffer.get_data(), read_size);
      channel->get_writer().write(buffer);
      channel->get_connection().close();
      return received;
    });
    auto client_channel = TcpSocketChannel(server_address);
    auto message = std::string("hello tcp");
    auto send_buffer = SharedBuffer(message.data(), message.size());
    client_channel.get_writer().write(send_buffer);
    auto receive_buffer = SharedBuffer();
    auto read_size = client_channel.get_reader().read(out(receive_buffer));
    REQUIRE(read_size == message.size());
    auto received = std::string(receive_buffer.get_data(), read_size);
    REQUIRE(received == message);
    auto server_received = server_future.get();
    REQUIRE(server_received == message);
    client_channel.get_connection().close();
    REQUIRE_THROWS_AS(client_channel.get_reader().read(out(receive_buffer)),
      EndOfFileException);
  }

  TEST_CASE("streaming_multiple_messages") {
    const auto MESSAGE_COUNT = 10;
    auto server_address = IpAddress("127.0.0.1", 15021);
    auto server = TcpServerSocket(server_address);
    auto server_future = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      auto accumulator = std::string(); 
      auto received_messages = std::vector<std::string>();
      while(received_messages.size() < MESSAGE_COUNT) {
        auto buffer = SharedBuffer();
        auto read_size = channel->get_reader().read(out(buffer));
        accumulator.append(buffer.get_data(), read_size);
        auto position = std::size_t(0);
        while((position = accumulator.find('\n')) != std::string::npos) {
          auto message = accumulator.substr(0, position);
          received_messages.push_back(message);
          auto echo = message + '\n';
          channel->get_writer().write(from<SharedBuffer>(echo));
          accumulator.erase(0, position + 1);
        }
      }
      return received_messages;
    });
    auto client_channel = TcpSocketChannel(server_address);
    for(auto i = 0; i < MESSAGE_COUNT; ++i) {
      auto payload = std::string("message") + std::to_string(i) + '\n';
      client_channel.get_writer().write(from<SharedBuffer>(payload));
    }
    auto echoed_messages = std::vector<std::string>();
    auto accumulator = std::string();
    while(echoed_messages.size() < MESSAGE_COUNT) {
      auto buffer = SharedBuffer();
      auto read_size = client_channel.get_reader().read(out(buffer));
      accumulator.append(buffer.get_data(), read_size);
      auto position = std::size_t(0);
      while((position = accumulator.find('\n')) != std::string::npos) {
        auto message = accumulator.substr(0, position);
        echoed_messages.push_back(message);
        accumulator.erase(0, position + 1);
      }
    }
    auto server_received = server_future.get();
    REQUIRE(echoed_messages.size() == static_cast<std::size_t>(MESSAGE_COUNT));
    REQUIRE(server_received.size() == static_cast<std::size_t>(MESSAGE_COUNT));
    for(auto i = 0; i < MESSAGE_COUNT; ++i) {
      auto expected = std::string("message") + std::to_string(i);
      REQUIRE(echoed_messages[i] == expected);
      REQUIRE(server_received[i] == expected);
    }
  }

  TEST_CASE("partial_read_and_buffer_management") {
    auto server_address = IpAddress("127.0.0.1", 15022);
    auto server = TcpServerSocket(server_address);
    const auto PAYLOAD_SIZE = 64 * 1024;
    const auto CHUNK_SIZE = 1024;
    auto server_future = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      auto payload = std::string(PAYLOAD_SIZE, 'z');
      channel->get_writer().write(from<SharedBuffer>(payload));
      return payload;
    });
    auto client_channel = TcpSocketChannel(server_address);
    auto total_read = std::size_t(0);
    auto reconstructed = std::string();
    while(total_read < PAYLOAD_SIZE) {
      auto buffer = SharedBuffer();
      auto read_size = client_channel.get_reader().read(
        out(buffer), CHUNK_SIZE);
      REQUIRE(read_size > 0);
      REQUIRE(read_size <= CHUNK_SIZE);
      reconstructed.append(buffer.get_data(), read_size);
      total_read += read_size;
    }
    REQUIRE(total_read == PAYLOAD_SIZE);
    auto server_payload = server_future.get();
    REQUIRE(reconstructed == server_payload);
  }

  TEST_CASE("connection_interruption_mid_transfer") {
    auto server_address = IpAddress("127.0.0.1", 15023);
    auto server = TcpServerSocket(server_address);
    const auto PAYLOAD_SIZE = 256 * 1024;
    const auto SERVER_SEND_SIZE = PAYLOAD_SIZE / 2;
    auto server_future = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      auto payload = std::string(SERVER_SEND_SIZE, 'p');
      channel->get_writer().write(from<SharedBuffer>(payload));
      return payload.size();
    });
    auto client_channel = TcpSocketChannel(server_address);
    auto total_read = std::size_t(0);
    auto reconstructed = std::string();
    auto encountered_eof = false;
    while(true) {
      try {
        auto buffer = SharedBuffer();
        auto read_size = client_channel.get_reader().read(out(buffer));
        if(read_size == 0) {
          encountered_eof = true;
          break;
        }
        reconstructed.append(buffer.get_data(), read_size);
        total_read += read_size;
        if(total_read >= PAYLOAD_SIZE) {
          break;
        }
      } catch(const EndOfFileException&) {
        encountered_eof = true;
        break;
      }
    }
    auto server_sent = server_future.get();
    REQUIRE(encountered_eof);
    REQUIRE(total_read == server_sent);
    REQUIRE(reconstructed.size() == server_sent);
  }

  TEST_CASE("connection_refused_and_connect_errors") {
    auto bad_address = IpAddress("127.0.0.1", 55333);
    REQUIRE_THROWS_AS((void)TcpSocketChannel(bad_address), ConnectException);
  }

  TEST_CASE("tcp_options_no_delay_and_write_buffer_size_smoke") {
    const auto MESSAGE_COUNT = 50;
    auto server_address = IpAddress("127.0.0.1", 15024);
    auto options = TcpSocketOptions();
    options.m_no_delay_enabled = true;
    options.m_write_buffer_size = 1024;
    auto server = TcpServerSocket(server_address, options);
    auto server_future = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      auto accumulator = std::string();
      auto received_messages = std::vector<std::string>();
      while(received_messages.size() < MESSAGE_COUNT) {
        auto buffer = SharedBuffer();
        auto read_size = channel->get_reader().read(out(buffer));
        accumulator.append(buffer.get_data(), read_size);
        auto position = std::size_t(0);
        while((position = accumulator.find('\n')) != std::string::npos) {
          auto message = accumulator.substr(0, position);
          received_messages.push_back(message);
          auto echo = message + '\n';
          channel->get_writer().write(from<SharedBuffer>(echo));
          accumulator.erase(0, position + 1);
        }
      }
      return received_messages;
    });
    auto client_channel = TcpSocketChannel(server_address, options);
    for(auto i = 0; i < MESSAGE_COUNT; ++i) {
      auto payload = std::string("m") + std::to_string(i) + '\n';
      client_channel.get_writer().write(from<SharedBuffer>(payload));
    }
    auto echoed_messages = std::vector<std::string>();
    auto accumulator = std::string();
    while(echoed_messages.size() < MESSAGE_COUNT) {
      auto buffer = SharedBuffer();
      auto read_size = client_channel.get_reader().read(out(buffer));
      accumulator.append(buffer.get_data(), read_size);
      auto position = std::size_t(0);
      while((position = accumulator.find('\n')) != std::string::npos) {
        auto message = accumulator.substr(0, position);
        echoed_messages.push_back(message);
        accumulator.erase(0, position + 1);
      }
    }
    auto server_received = server_future.get();
    REQUIRE(echoed_messages.size() == MESSAGE_COUNT);
    REQUIRE(server_received.size() == MESSAGE_COUNT);
    for(auto i = 0; i < MESSAGE_COUNT; ++i) {
      auto expected = std::string("m") + std::to_string(i);
      REQUIRE(echoed_messages[i] == expected);
      REQUIRE(server_received[i] == expected);
    }
  }

  TEST_CASE("half_open_detection_server_side") {
    auto server_address = IpAddress("127.0.0.1", 15025);
    auto server = TcpServerSocket(server_address);
    auto server_future = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      auto buffer = SharedBuffer();
      try {
        channel->get_reader().read(out(buffer));
        return false;
      } catch(const EndOfFileException&) {
        return true;
      }
    });
    auto client_channel = TcpSocketChannel(server_address);
    client_channel.get_connection().close();
    auto server_detected = server_future.get();
    REQUIRE(server_detected);
  }
}

TEST_SUITE("MulticastSocket") {
  TEST_CASE("join_group_and_simple_send_receive") {
    auto group = IpAddress("239.255.0.1", 16000);
    auto interface = IpAddress("127.0.0.1", 0);
    auto options = MulticastSocketOptions();
    options.m_enable_loopback = true;
    options.m_max_datagram_size = 4096;
    options.m_receive_buffer_size = 8192;
    auto listener = MulticastSocketChannel(group, interface, options);
    auto sender = MulticastSocketChannel(group, interface, options);
    auto message = std::string("multicast hello");
    auto send_buffer = from<SharedBuffer>(message);
    sender.get_writer().write(send_buffer);
    REQUIRE(listener.get_reader().poll());
    auto receive_buffer = SharedBuffer();
    REQUIRE(listener.get_reader().read(out(receive_buffer)) == message.size());
    REQUIRE(receive_buffer == message);
  }

  TEST_CASE("multiple_listeners_receive_same_datagram") {
    auto group = IpAddress("239.255.0.1", 16001);
    auto interface = IpAddress("127.0.0.1", 0);
    auto options = MulticastSocketOptions();
    options.m_enable_loopback = true;
    options.m_max_datagram_size = 2048;
    options.m_receive_buffer_size = 8192;
    auto listener1 = MulticastSocketChannel(group, interface, options);
    auto listener2 = MulticastSocketChannel(group, interface, options);
    auto sender = MulticastSocketChannel(group, interface, options);
    auto message = std::string("multicast to many");
    auto send_buffer = from<SharedBuffer>(message);
    sender.get_writer().write(send_buffer);
    REQUIRE(listener1.get_reader().poll());
    REQUIRE(listener2.get_reader().poll());
    auto receive_buffer1 = SharedBuffer();
    auto receive_buffer2 = SharedBuffer();
    REQUIRE(
      listener1.get_reader().read(out(receive_buffer1)) == message.size());
    REQUIRE(
      listener2.get_reader().read(out(receive_buffer2)) == message.size());
    REQUIRE(receive_buffer1 == message);
    REQUIRE(receive_buffer2 == message);
  }

  TEST_CASE("loopback_option_affects_local_receipt") {
    auto group = IpAddress("239.255.0.2", 16002);
    auto interface = IpAddress("127.0.0.1", 0);
    auto options_loop = MulticastSocketOptions();
    options_loop.m_enable_loopback = true;
    options_loop.m_max_datagram_size = 1024;
    options_loop.m_receive_buffer_size = 4096;
    auto listener_a = MulticastSocketChannel(group, interface, options_loop);
    auto sender_a = MulticastSocketChannel(group, interface, options_loop);
    auto message = std::string("loopback on");
    sender_a.get_writer().write(from<SharedBuffer>(message));
    REQUIRE(listener_a.get_reader().poll());
    REQUIRE(sender_a.get_reader().poll());
    auto receive_buffer_a = SharedBuffer();
    auto receive_buffer_sender = SharedBuffer();
    REQUIRE(
      listener_a.get_reader().read(out(receive_buffer_a)) == message.size());
    REQUIRE(
      sender_a.get_reader().read(out(receive_buffer_sender)) == message.size());
    REQUIRE(receive_buffer_a == message);
    REQUIRE(receive_buffer_sender == message);
    auto options_no_loop = MulticastSocketOptions();
    options_no_loop.m_enable_loopback = false;
    options_no_loop.m_max_datagram_size = 1024;
    options_no_loop.m_receive_buffer_size = 4096;
    auto listener_b = MulticastSocketChannel(group, interface, options_no_loop);
    auto sender_b = MulticastSocketChannel(group, interface, options_no_loop);
    auto message2 = std::string("loopback off");
    sender_b.get_writer().write(from<SharedBuffer>(message2));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(!listener_b.get_reader().poll());
    REQUIRE(!sender_b.get_reader().poll());
  }

  TEST_CASE("ttl_option_allows_local_delivery") {
    auto group = IpAddress("239.255.0.3", 16003);
    auto interface = IpAddress("127.0.0.1", 0);
    auto options_ttl = MulticastSocketOptions();
    options_ttl.m_enable_loopback = true;
    options_ttl.m_ttl = 0;
    options_ttl.m_max_datagram_size = 1024;
    options_ttl.m_receive_buffer_size = 4096;
    auto listener = MulticastSocketChannel(group, interface, options_ttl);
    auto sender = MulticastSocketChannel(group, interface, options_ttl);
    auto message = std::string("ttl test");
    sender.get_writer().write(from<SharedBuffer>(message));
    REQUIRE(listener.get_reader().poll());
    auto receive_buffer = SharedBuffer();
    REQUIRE(listener.get_reader().read(out(receive_buffer)) == message.size());
    REQUIRE(receive_buffer == message);
  }

  TEST_CASE("leave_group_and_no_more_receives") {
    auto group = IpAddress("239.255.0.4", 16004);
    auto interface = IpAddress("127.0.0.1", 0);
    auto options = MulticastSocketOptions();
    options.m_enable_loopback = true;
    options.m_max_datagram_size = 1024;
    options.m_receive_buffer_size = 4096;
    auto listener1 = MulticastSocketChannel(group, interface, options);
    auto listener2 = MulticastSocketChannel(group, interface, options);
    auto sender = MulticastSocketChannel(group, interface, options);
    auto first_message = std::string("first");
    sender.get_writer().write(from<SharedBuffer>(first_message));
    REQUIRE(listener1.get_reader().poll());
    REQUIRE(listener2.get_reader().poll());
    auto buffer1 = SharedBuffer();
    auto buffer2 = SharedBuffer();
    REQUIRE(listener1.get_reader().read(out(buffer1)) == first_message.size());
    REQUIRE(listener2.get_reader().read(out(buffer2)) == first_message.size());
    REQUIRE(buffer1 == first_message);
    REQUIRE(buffer2 == first_message);
    listener1.get_connection().close();
    auto second_message = std::string("second");
    sender.get_writer().write(from<SharedBuffer>(second_message));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(!listener1.get_reader().poll());
    REQUIRE(listener2.get_reader().poll());
    auto buffer_2b = SharedBuffer();
    REQUIRE(
      listener2.get_reader().read(out(buffer_2b)) == second_message.size());
    REQUIRE(buffer_2b == second_message);
  }

  TEST_CASE("concurrent_senders_and_multiple_receivers") {
    const auto SENDER_COUNT = 3;
    const auto MESSAGES_PER_SENDER = 20;
    auto group = IpAddress("239.255.0.5", 16005);
    auto interface = IpAddress("127.0.0.1", 0);
    auto options = MulticastSocketOptions();
    options.m_enable_loopback = true;
    options.m_max_datagram_size = 512;
    options.m_receive_buffer_size = 8192;
    auto receiver_count = 2;
    auto receivers = std::vector<std::unique_ptr<MulticastSocketChannel>>();
    for(auto i = 0; i < receiver_count; ++i) {
      receivers.push_back(
        std::make_unique<MulticastSocketChannel>(group, interface, options));
    }
    auto sender_futures = std::vector<std::future<void>>();
    for(auto s = 0; s < SENDER_COUNT; ++s) {
      sender_futures.push_back(std::async(std::launch::async, [&, s] {
        auto sender = MulticastSocketChannel(group, interface, options);
        for(auto m = 0; m < MESSAGES_PER_SENDER; ++m) {
          auto payload =
            std::string("s") + std::to_string(s) + ":" + std::to_string(m);
          sender.get_writer().write(from<SharedBuffer>(payload));
        }
      }));
    }
    auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(2000);
    auto received_sets = std::vector<std::set<std::string>>(receiver_count);
    while(std::chrono::steady_clock::now() < deadline) {
      for(auto i = std::size_t(0); i < receivers.size(); ++i) {
        while(receivers[i]->get_reader().poll()) {
          auto buf = SharedBuffer();
          auto read_size = receivers[i]->get_reader().read(out(buf));
          if(read_size == 0) {
            continue;
          }
          received_sets[i].insert(std::string(buf.get_data(), read_size));
        }
      }
      auto all_have_some = true;
      for(auto& set : received_sets) {
        if(set.empty()) {
          all_have_some = false;
          break;
        }
      }
      if(all_have_some) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    for(auto& future : sender_futures) {
      future.get();
    }
    for(auto& set : received_sets) {
      REQUIRE(!set.empty());
    }
  }

  TEST_CASE("invalid_group_and_error_handling") {
    auto bad_group = IpAddress("127.0.0.1", 16006);
    auto interface = IpAddress("127.0.0.1", 0);
    try {
      auto channel = MulticastSocketChannel(bad_group, interface);
      channel.get_connection().close();
      FAIL("Expected exception constructing MulticastSocketChannel with non-multicast group");
    } catch(...) {
    }
  }

  TEST_CASE("bind_to_interface_variants") {
    auto group = IpAddress("239.255.0.6", 16006);
    auto loopback_interface = IpAddress("127.0.0.1", 0);
    auto wildcard_interface = IpAddress("0.0.0.0", 0);
    auto options = MulticastSocketOptions();
    options.m_enable_loopback = true;
    options.m_max_datagram_size = 1024;
    options.m_receive_buffer_size = 8192;
    auto wild_listener =
      MulticastSocketChannel(group, wildcard_interface, options);
    auto loop_listener =
      MulticastSocketChannel(group, loopback_interface, options);
    auto sender = MulticastSocketChannel(group, loopback_interface, options);
    auto message = std::string("interface test");
    sender.get_writer().write(from<SharedBuffer>(message));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    REQUIRE(wild_listener.get_reader().poll());
    REQUIRE(loop_listener.get_reader().poll());
    auto wild_buffer = SharedBuffer();
    auto loop_buffer = SharedBuffer();
    REQUIRE(
      wild_listener.get_reader().read(out(wild_buffer)) == message.size());
    REQUIRE(
      loop_listener.get_reader().read(out(loop_buffer)) == message.size());
    REQUIRE(wild_buffer == message);
    REQUIRE(loop_buffer == message);
  }
}
