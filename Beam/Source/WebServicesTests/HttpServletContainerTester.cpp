#include <doctest/doctest.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/HttpServletContainer.hpp"

using namespace Beam;

namespace {
  template<typename C>
  class TestHttpServlet {
    public:
      using Container = C;

      std::vector<HttpRequestSlot> get_slots() {
        auto slots = std::vector<HttpRequestSlot>();
        slots.push_back({
          [] (const auto& request) {
            return request.get_uri().get_path() == "/test" &&
              request.get_method() == HttpMethod::GET;
          },
          [this] (const auto& request) {
            return handle_get_test(request);
          }
        });
        slots.push_back({
          [] (const auto& request) {
            return request.get_uri().get_path() == "/echo" &&
              request.get_method() == HttpMethod::POST;
          },
          [this] (const auto& request) {
            return handle_post_echo(request);
          }
        });
        return slots;
      }

      void close() {
        m_is_closed = true;
      }

    private:
      bool m_is_closed;

      HttpResponse handle_get_test(const HttpRequest& request) {
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_header(HttpHeader("Content-Type", "text/plain"));
        response.set_body(from<SharedBuffer>("Test successful"));
        return response;
      }

      HttpResponse handle_post_echo(const HttpRequest& request) {
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_header(
          HttpHeader("Content-Type", "application/octet-stream"));
        response.set_body(request.get_body());
        return response;
      }
  };

  struct MetaTestHttpServlet {
    template<typename C>
    struct apply {
      using type = TestHttpServlet<C>;
    };
  };

  using TestHttpServletContainer =
    HttpServletContainer<MetaTestHttpServlet, LocalServerConnection*>;

  struct Fixture {
    LocalServerConnection m_server_connection;
    TestHttpServletContainer m_container;

    Fixture()
      : m_container(init(), &m_server_connection) {}
  };
}

TEST_SUITE("HttpServletContainer") {
  TEST_CASE_FIXTURE(Fixture, "handle_get_request") {
    auto client = LocalClientChannel("http", m_server_connection);
    auto request_text = std::string(
      "GET /test HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 200 OK") != std::string::npos);
    REQUIRE(
      response_text.find("Content-Type: text/plain") != std::string::npos);
    REQUIRE(response_text.find("Test successful") != std::string::npos);
  }

  TEST_CASE_FIXTURE(Fixture, "handle_post_echo_request") {
    auto client = LocalClientChannel("http", m_server_connection);
    auto body = std::string("Echo this message");
    auto request_text = std::string(
      "POST /echo HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: ") + std::to_string(body.size()) + "\r\n"
      "Connection: keep-alive\r\n"
      "\r\n" + body;
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 200 OK") != std::string::npos);
    REQUIRE(response_text.find("Echo this message") != std::string::npos);
  }

  TEST_CASE_FIXTURE(Fixture, "handle_404_for_unknown_path") {
    auto client = LocalClientChannel("http", m_server_connection);
    auto request_text = std::string(
      "GET /unknown HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 404 Not Found") != std::string::npos);
  }

  TEST_CASE_FIXTURE(Fixture, "handle_multiple_sequential_requests") {
    auto client = LocalClientChannel("http", m_server_connection);
    for(auto i = 0; i < 3; ++i) {
      auto request_text = std::string(
        "GET /test HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 0\r\n"
        "Connection: keep-alive\r\n"
        "\r\n");
      client.get_writer().write(from<SharedBuffer>(request_text));
      auto buffer = SharedBuffer();
      client.get_reader().read(out(buffer));
      auto response_text = std::string(buffer.get_data(), buffer.get_size());
      REQUIRE(response_text.find("HTTP/1.1 200 OK") != std::string::npos);
      REQUIRE(response_text.find("Test successful") != std::string::npos);
      reset(buffer);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "handle_request_with_query_parameters") {
    auto client = LocalClientChannel("http", m_server_connection);
    auto request_text = std::string(
      "GET /test?param=value HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 200 OK") != std::string::npos);
    REQUIRE(response_text.find("Test successful") != std::string::npos);
  }

  TEST_CASE_FIXTURE(Fixture, "handle_connection_close") {
    auto client = LocalClientChannel("http", m_server_connection);
    auto request_text = std::string(
      "GET /test HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: close\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 200 OK") != std::string::npos);
  }
}
