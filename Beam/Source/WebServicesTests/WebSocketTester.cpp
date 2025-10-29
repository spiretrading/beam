#include <future>
#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/WebServices/WebSocket.hpp"

using namespace Beam;

TEST_SUITE("WebSocket") {
  TEST_CASE("connect_and_send_text_frame") {
    auto server = LocalServerConnection();
    auto config = WebSocketConfig();
    config.set_uri("ws://example.com/socket");
    auto client_task = std::async(std::launch::async, [&] {
      auto socket = WebSocket(std::move(config), [&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("ws", server);
      });
      socket.write(from<SharedBuffer>("Hello, WebSocket!"));
      return socket.read();
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("GET /socket HTTP/1.1") != std::string::npos);
    REQUIRE(request_text.find("Upgrade: websocket") != std::string::npos);
    REQUIRE(request_text.find("Connection: Upgrade") != std::string::npos);
    REQUIRE(request_text.find("Sec-WebSocket-Key: ") != std::string::npos);
    REQUIRE(
      request_text.find("Sec-WebSocket-Version: 13") != std::string::npos);
    auto key_pos = request_text.find("Sec-WebSocket-Key: ");
    auto key_end = request_text.find("\r\n", key_pos);
    auto key = request_text.substr(key_pos + 19, key_end - key_pos - 19);
    auto accept_key = encode_base64(from<SharedBuffer>(
      Details::compute_sha_digest(
        key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ") + accept_key + "\r\n\r\n";
    channel->get_writer().write(from<SharedBuffer>(response_text));
    reset(buffer);
    channel->get_reader().read(out(buffer));
    REQUIRE(buffer.get_size() >= 2);
    auto frame_header = buffer.get_data();
    auto is_final = (frame_header[0] & 0x80) != 0;
    auto opcode = frame_header[0] & 0x0F;
    auto is_masked = (frame_header[1] & 0x80) != 0;
    auto payload_length = frame_header[1] & 0x7F;
    REQUIRE(is_final);
    REQUIRE(opcode == 1);
    REQUIRE(is_masked);
    REQUIRE(payload_length == 17);
    auto masking_key = std::array<unsigned char, 4>();
    std::memcpy(masking_key.data(), buffer.get_data() + 2, 4);
    auto payload = std::string(17, '\0');
    for(auto i = std::size_t(0); i < 17; ++i) {
      payload[i] = buffer.get_data()[6 + i] ^ masking_key[i % 4];
    }
    REQUIRE(payload == "Hello, WebSocket!");
    reset(buffer);
    auto response_frame = SharedBuffer();
    auto response_header = std::array<unsigned char, 2>{0x81, 0x0E};
    append(response_frame, response_header.data(), response_header.size());
    auto response_payload = std::string("Hello, Client!");
    append(response_frame, response_payload.data(), response_payload.size());
    channel->get_writer().write(response_frame);
    auto received_message = client_task.get();
    auto received_text = std::string(
      received_message.get_data(), received_message.get_size());
    REQUIRE(received_text == "Hello, Client!");
  }

  TEST_CASE("connect_with_custom_protocols") {
    auto server = LocalServerConnection();
    auto config = WebSocketConfig();
    config.set_uri("ws://example.com/chat");
    config.set_protocols({"chat", "superchat"});
    auto client_task = std::async(std::launch::async, [&] {
      auto socket = WebSocket(std::move(config), [&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("ws", server);
      });
      return socket.get_uri();
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("Sec-WebSocket-Protocol: chat, superchat") !=
      std::string::npos);
    auto key_pos = request_text.find("Sec-WebSocket-Key: ");
    auto key_end = request_text.find("\r\n", key_pos);
    auto key = request_text.substr(key_pos + 19, key_end - key_pos - 19);
    auto accept_key = encode_base64(from<SharedBuffer>(
      Details::compute_sha_digest(
        key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
    auto response_text = std::string(
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ") + accept_key + "\r\n\r\n";
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto uri = client_task.get();
    REQUIRE(uri.get_path() == "/chat");
  }

  TEST_CASE("send_large_message") {
    auto server = LocalServerConnection();
    auto config = WebSocketConfig();
    config.set_uri("ws://example.com/data");
    auto large_message = std::string(300, 'A');
    auto client_task = std::async(std::launch::async, [&] {
      auto socket = WebSocket(std::move(config), [&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("ws", server);
      });
      socket.write(from<SharedBuffer>(large_message));
      return socket.read();
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto key_pos = buffer.get_data();
    auto key_start = std::string(key_pos, buffer.get_size()).find(
      "Sec-WebSocket-Key: ");
    auto key_line_end = std::string(key_pos, buffer.get_size()).find(
      "\r\n", key_start);
    auto key = std::string(key_pos, buffer.get_size()).substr(
      key_start + 19, key_line_end - key_start - 19);
    auto accept_key = encode_base64(from<SharedBuffer>(
      Details::compute_sha_digest(
        key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ") + accept_key + "\r\n\r\n";
    channel->get_writer().write(from<SharedBuffer>(response_text));
    reset(buffer);
    channel->get_reader().read(out(buffer));
    auto frame_header = buffer.get_data();
    auto is_masked = (frame_header[1] & 0x80) != 0;
    auto payload_len_indicator = frame_header[1] & 0x7F;
    REQUIRE(is_masked);
    REQUIRE(payload_len_indicator == 126);
    auto extended_length = std::uint16_t();
    std::memcpy(&extended_length, frame_header + 2, 2);
    extended_length = boost::endian::big_to_native(extended_length);
    REQUIRE(extended_length == 300);
    reset(buffer);
    auto response_frame = SharedBuffer();
    append(response_frame, std::uint8_t(0x81));
    append(response_frame, std::uint8_t(126));
    auto response_length = boost::endian::native_to_big(std::uint16_t(5));
    append(response_frame, response_length);
    append(response_frame, "LARGE", 5);
    channel->get_writer().write(response_frame);
    auto received_message = client_task.get();
    auto received_text = std::string(
      received_message.get_data(), received_message.get_size());
    REQUIRE(received_text == "LARGE");
  }

  TEST_CASE("send_multiple_messages") {
    auto server = LocalServerConnection();
    auto config = WebSocketConfig();
    config.set_uri("ws://example.com/multi");
    auto client_task = std::async(std::launch::async, [&] {
      auto socket = WebSocket(std::move(config), [&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("ws", server);
      });
      socket.write(from<SharedBuffer>("Message 1"));
      socket.write(from<SharedBuffer>("Message 2"));
      auto response1 = socket.read();
      auto response2 = socket.read();
      return std::make_pair(
        std::string(response1.get_data(), response1.get_size()),
        std::string(response2.get_data(), response2.get_size()));
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto key_pos = buffer.get_data();
    auto key_start = std::string(key_pos, buffer.get_size()).find(
      "Sec-WebSocket-Key: ");
    auto key_line_end = std::string(key_pos, buffer.get_size()).find(
      "\r\n", key_start);
    auto key = std::string(key_pos, buffer.get_size()).substr(
      key_start + 19, key_line_end - key_start - 19);
    auto accept_key = encode_base64(from<SharedBuffer>(
      Details::compute_sha_digest(
        key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ") + accept_key + "\r\n\r\n";
    channel->get_writer().write(from<SharedBuffer>(response_text));
    reset(buffer);
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_reader().read(out(buffer));
    auto response_frame1 = SharedBuffer();
    append(response_frame1, std::uint8_t(0x81));
    append(response_frame1, std::uint8_t(8));
    append(response_frame1, "Reply #1", 8);
    channel->get_writer().write(response_frame1);
    auto response_frame2 = SharedBuffer();
    append(response_frame2, std::uint8_t(0x81));
    append(response_frame2, std::uint8_t(8));
    append(response_frame2, "Reply #2", 8);
    channel->get_writer().write(response_frame2);
    auto responses = client_task.get();
    REQUIRE(responses.first == "Reply #1");
    REQUIRE(responses.second == "Reply #2");
  }

  TEST_CASE("connect_with_custom_port") {
    auto server = LocalServerConnection();
    auto config = WebSocketConfig();
    config.set_uri("ws://example.com:8080/socket");
    auto client_task = std::async(std::launch::async, [&] {
      auto socket = WebSocket(std::move(config), [&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("ws", server);
      });
      return socket.get_uri();
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("Host: example.com:8080") != std::string::npos);
    auto key_pos = request_text.find("Sec-WebSocket-Key: ");
    auto key_end = request_text.find("\r\n", key_pos);
    auto key = request_text.substr(key_pos + 19, key_end - key_pos - 19);
    auto accept_key = encode_base64(from<SharedBuffer>(
      Details::compute_sha_digest(
        key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
    auto response_text = std::string(
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ") + accept_key + "\r\n\r\n";
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto uri = client_task.get();
    REQUIRE(uri.get_port() == 8080);
  }

  TEST_CASE("send_empty_message") {
    auto server = LocalServerConnection();
    auto config = WebSocketConfig();
    config.set_uri("ws://example.com/empty");
    auto client_task = std::async(std::launch::async, [&] {
      auto socket = WebSocket(std::move(config), [&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("ws", server);
      });
      socket.write(from<SharedBuffer>(""));
      return socket.read();
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto key_pos = buffer.get_data();
    auto key_start = std::string(key_pos, buffer.get_size()).find(
      "Sec-WebSocket-Key: ");
    auto key_line_end = std::string(key_pos, buffer.get_size()).find(
      "\r\n", key_start);
    auto key = std::string(key_pos, buffer.get_size()).substr(
      key_start + 19, key_line_end - key_start - 19);
    auto accept_key = encode_base64(from<SharedBuffer>(
      Details::compute_sha_digest(
        key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ") + accept_key + "\r\n\r\n";
    channel->get_writer().write(from<SharedBuffer>(response_text));
    reset(buffer);
    channel->get_reader().read(out(buffer));
    auto frame_header = buffer.get_data();
    auto payload_length = frame_header[1] & 0x7F;
    REQUIRE(payload_length == 0);
    reset(buffer);
    auto response_frame = SharedBuffer();
    append(response_frame, std::uint8_t(0x81));
    append(response_frame, std::uint8_t(0));
    channel->get_writer().write(response_frame);
    auto received_message = client_task.get();
    REQUIRE(received_message.get_size() == 0);
  }
}
