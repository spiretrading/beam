module;
#include "Prelude.hpp"
#include <array>
#include <future>
#include <string>
#include <boost/optional/optional_io.hpp>
#include <zlib.h>
#include "Beam/IO/SharedBuffer.hpp"
#include <doctest/doctest.h>

module Beam;

using namespace Beam;

TEST_SUITE("HttpClient") {
  TEST_CASE("send_basic_get_request") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
        return std::make_unique<LocalClientChannel>("http", server);
      });
    auto request = HttpRequest("http://example.com/test");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("GET /test HTTP/1.1") != std::string::npos);
    REQUIRE(request_text.find("Host: example.com") != std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 13\r\n"
      "\r\n"
      "Hello, World!");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto body = std::string(
      response.get_body().get_data(), response.get_body().get_size());
    REQUIRE(body == "Hello, World!");
  }

  TEST_CASE("send_post_request_with_body") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto body = from<SharedBuffer>("{\"key\":\"value\"}");
    auto request =
      HttpRequest(HttpMethod::POST, "http://example.com/api", body);
    request.add(HttpHeader("Content-Type", "application/json"));
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("POST /api HTTP/1.1") != std::string::npos);
    REQUIRE(request_text.find("Host: example.com") != std::string::npos);
    REQUIRE(
      request_text.find("Content-Type: application/json") != std::string::npos);
    REQUIRE(request_text.find("{\"key\":\"value\"}") != std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 201 Created\r\n"
      "Content-Length: 0\r\n"
      "\r\n");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::CREATED);
  }

  TEST_CASE("send_request_with_query_parameters") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/search?q=test&page=1");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("GET /search?q=test&page=1 HTTP/1.1") !=
      std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 7\r\n"
      "\r\n"
      "Results");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
  }

  TEST_CASE("handle_404_not_found") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/missing");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 9\r\n"
      "\r\n"
      "Not Found");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::NOT_FOUND);
    auto body = std::string(
      response.get_body().get_data(), response.get_body().get_size());
    REQUIRE(body == "Not Found");
  }

  TEST_CASE("send_request_with_custom_headers") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/data");
    request.add(HttpHeader("Accept", "application/json"));
    request.add(HttpHeader("User-Agent", "TestClient/1.0"));
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("Accept: application/json") != std::string::npos);
    REQUIRE(
      request_text.find("User-Agent: TestClient/1.0") != std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "{}");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
  }

  TEST_CASE("handle_redirect_response") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/old");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 301 Moved Permanently\r\n"
      "Location: http://example.com/new\r\n"
      "Content-Length: 0\r\n"
      "\r\n");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::MOVED_PERMANENTLY);
    auto location = response.get_header("Location");
    REQUIRE(location);
    REQUIRE(*location == "http://example.com/new");
  }

  TEST_CASE("send_request_with_cookies") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/page");
    request.add(Cookie("session_id", "abc123"));
    request.add(Cookie("user", "john"));
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(request_text.find("Cookie: ") != std::string::npos);
    REQUIRE(request_text.find("session_id=abc123") != std::string::npos);
    REQUIRE(request_text.find("user=john") != std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "\r\n");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
  }

  TEST_CASE("receive_response_with_cookies") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/login");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Set-Cookie: session_id=xyz789; Path=/\r\n"
      "Set-Cookie: token=secret; HttpOnly\r\n"
      "Content-Length: 0\r\n"
      "\r\n");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto& cookies = response.get_cookies();
    REQUIRE(cookies.size() == 2);
    REQUIRE(cookies[0].get_name() == "session_id");
    REQUIRE(cookies[0].get_value() == "xyz789");
    REQUIRE(cookies[1].get_name() == "token");
    REQUIRE(cookies[1].get_value() == "secret");
  }

  TEST_CASE("accept_encoding_header_added") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/test");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(
      request_text.find("Accept-Encoding: gzip, deflate") != std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "\r\n");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    client_task.get();
  }

  TEST_CASE("accept_encoding_header_not_overridden") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/test");
    request.add(HttpHeader("Accept-Encoding", "identity"));
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    auto request_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(
      request_text.find("Accept-Encoding: identity") != std::string::npos);
    REQUIRE(request_text.find("Accept-Encoding: gzip") == std::string::npos);
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "\r\n");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    client_task.get();
  }

  TEST_CASE("decompress_gzip_response") {
    auto plain = std::string("Hello, compressed world!");
    auto compressed = std::string();
    {
      auto stream = z_stream();
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS,
        8, Z_DEFAULT_STRATEGY);
      auto chunk = std::array<char, 256>();
      stream.avail_in = static_cast<uInt>(plain.size());
      stream.next_in = reinterpret_cast<Bytef*>(plain.data());
      stream.avail_out = static_cast<uInt>(chunk.size());
      stream.next_out = reinterpret_cast<Bytef*>(chunk.data());
      deflate(&stream, Z_FINISH);
      compressed.assign(chunk.data(), chunk.size() - stream.avail_out);
      deflateEnd(&stream);
    }
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/data");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    reset(buffer);
    auto header = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Encoding: gzip\r\n"
      "Content-Length: " + std::to_string(compressed.size()) + "\r\n"
      "\r\n");
    auto response_buffer = SharedBuffer();
    append(response_buffer, header.data(), header.size());
    append(response_buffer, compressed.data(), compressed.size());
    channel->get_writer().write(response_buffer);
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto body = std::string(
      response.get_body().get_data(), response.get_body().get_size());
    REQUIRE(body == plain);
  }

  TEST_CASE("decompress_deflate_response") {
    auto plain = std::string("Hello, deflated world!");
    auto compressed = std::string();
    {
      auto stream = z_stream();
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8,
        Z_DEFAULT_STRATEGY);
      auto chunk = std::array<char, 256>();
      stream.avail_in = static_cast<uInt>(plain.size());
      stream.next_in = reinterpret_cast<Bytef*>(plain.data());
      stream.avail_out = static_cast<uInt>(chunk.size());
      stream.next_out = reinterpret_cast<Bytef*>(chunk.data());
      deflate(&stream, Z_FINISH);
      compressed.assign(chunk.data(), chunk.size() - stream.avail_out);
      deflateEnd(&stream);
    }
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/data");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    reset(buffer);
    auto header = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Encoding: deflate\r\n"
      "Content-Length: " + std::to_string(compressed.size()) + "\r\n"
      "\r\n");
    auto response_buffer = SharedBuffer();
    append(response_buffer, header.data(), header.size());
    append(response_buffer, compressed.data(), compressed.size());
    channel->get_writer().write(response_buffer);
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto body = std::string(
      response.get_body().get_data(), response.get_body().get_size());
    REQUIRE(body == plain);
  }

  TEST_CASE("uncompressed_response_unchanged") {
    auto server = LocalServerConnection();
    auto client = HttpClient([&] (const auto& uri) {
      return std::make_unique<LocalClientChannel>("http", server);
    });
    auto request = HttpRequest("http://example.com/plain");
    auto client_task = std::async(std::launch::async, [&] {
      return client.send(request);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_reader().read(out(buffer));
    reset(buffer);
    auto response_text = std::string(
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 5\r\n"
      "\r\n"
      "plain");
    channel->get_writer().write(from<SharedBuffer>(response_text));
    auto response = client_task.get();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto body = std::string(
      response.get_body().get_data(), response.get_body().get_size());
    REQUIRE(body == "plain");
  }
}
