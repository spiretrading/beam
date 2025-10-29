#include <future>
#include <string>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/HttpClient.hpp"

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
}
