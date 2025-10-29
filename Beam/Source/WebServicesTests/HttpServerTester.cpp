#include <future>
#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/HttpServer.hpp"

using namespace Beam;

TEST_SUITE("HttpServer") {
  TEST_CASE("handle_basic_get_request") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/test";
      },
      [] (const auto& request) {
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_header(HttpHeader("Content-Type", "text/plain"));
        response.set_body(from<SharedBuffer>("Hello, World!"));
        return response;
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
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
    REQUIRE(response_text.find("Hello, World!") != std::string::npos);
  }

  TEST_CASE("handle_post_request_with_body") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/api/data" &&
          request.get_method() == HttpMethod::POST;
      },
      [] (const auto& request) {
        auto body = std::string(
          request.get_body().get_data(), request.get_body().get_size());
        auto response = HttpResponse(HttpStatusCode::CREATED);
        response.set_header(HttpHeader("Content-Type", "application/json"));
        response.set_body(from<SharedBuffer>("{\"status\":\"created\"}"));
        return response;
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_body = std::string("{\"name\":\"test\"}");
    auto request_text = std::string(
      "POST /api/data HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: ") + std::to_string(request_body.size()) + "\r\n"
      "Connection: keep-alive\r\n"
      "\r\n" + request_body;
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 201 Created") != std::string::npos);
    REQUIRE(response_text.find("Content-Type: application/json") !=
      std::string::npos);
    REQUIRE(response_text.find("{\"status\":\"created\"}") !=
      std::string::npos);
  }

  TEST_CASE("return_404_for_unmatched_path") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/exists";
      },
      [] (const auto& request) {
        return HttpResponse(HttpStatusCode::OK);
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_text = std::string(
      "GET /does-not-exist HTTP/1.1\r\n"
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

  TEST_CASE("match_first_matching_slot") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/test";
      },
      [] (const auto& request) {
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_body(from<SharedBuffer>("First"));
        return response;
      }
    });
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/test";
      },
      [] (const auto& request) {
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_body(from<SharedBuffer>("Second"));
        return response;
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
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
    REQUIRE(response_text.find("First") != std::string::npos);
    REQUIRE(response_text.find("Second") == std::string::npos);
  }

  TEST_CASE("handle_slot_exception_with_500_response") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/error";
      },
      [] (const auto& request) -> HttpResponse {
        throw std::runtime_error("Internal error occurred");
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_text = std::string(
      "GET /error HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 500 Internal Server Error") !=
      std::string::npos);
    REQUIRE(response_text.find("Content-Type: application/json") !=
      std::string::npos);
    REQUIRE(response_text.find("Internal error occurred") != std::string::npos);
  }

  TEST_CASE("handle_multiple_requests_on_keep_alive_connection") {
    auto server_connection = LocalServerConnection();
    auto request_count = 0;
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return true;
      },
      [&] (const auto& request) {
        ++request_count;
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_body(
          from<SharedBuffer>("Request " + std::to_string(request_count)));
        return response;
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_text = std::string(
      "GET /first HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("Request 1") != std::string::npos);
    reset(buffer);
    request_text = std::string(
      "GET /second HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    client.get_reader().read(out(buffer));
    response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("Request 2") != std::string::npos);
    REQUIRE(request_count == 2);
  }

  TEST_CASE("close_connection_on_connection_close_header") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return true;
      },
      [] (const auto& request) {
        return HttpResponse(HttpStatusCode::OK);
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
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

  TEST_CASE("handle_request_with_query_parameters") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/search";
      },
      [] (const auto& request) {
        auto query = request.get_uri().get_query();
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_body(from<SharedBuffer>("Query: " + query));
        return response;
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_text = std::string(
      "GET /search?q=test&page=1 HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("Query: q=test&page=1") != std::string::npos);
  }

  TEST_CASE("handle_request_with_custom_headers") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/headers";
      },
      [] (const auto& request) {
        auto auth = request.get_header("Authorization");
        auto response = HttpResponse(HttpStatusCode::OK);
        if(auth) {
          response.set_body(from<SharedBuffer>("Authorized: " + *auth));
        } else {
          response.set_body(from<SharedBuffer>("No authorization"));
        }
        return response;
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_text = std::string(
      "GET /headers HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Authorization: Bearer token123\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("Authorized: Bearer token123") !=
      std::string::npos);
  }

  TEST_CASE("handle_different_http_methods") {
    auto server_connection = LocalServerConnection();
    auto slots = std::vector<HttpRequestSlot>();
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/resource" &&
          request.get_method() == HttpMethod::PUT;
      },
      [] (const auto& request) {
        auto response = HttpResponse(HttpStatusCode::OK);
        response.set_body(from<SharedBuffer>("Updated"));
        return response;
      }
    });
    slots.push_back({
      [] (const auto& request) {
        return request.get_uri().get_path() == "/resource" &&
          request.get_method() == HttpMethod::DELETE;
      },
      [] (const auto& request) {
        return HttpResponse(HttpStatusCode::NO_CONTENT);
      }
    });
    auto server = HttpServer(&server_connection, std::move(slots));
    auto client = LocalClientChannel("http", server_connection);
    auto request_text = std::string(
      "PUT /resource HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    auto buffer = SharedBuffer();
    client.get_reader().read(out(buffer));
    auto response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 200 OK") != std::string::npos);
    REQUIRE(response_text.find("Updated") != std::string::npos);
    reset(buffer);
    request_text = std::string(
      "DELETE /resource HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Content-Length: 0\r\n"
      "Connection: keep-alive\r\n"
      "\r\n");
    client.get_writer().write(from<SharedBuffer>(request_text));
    client.get_reader().read(out(buffer));
    response_text = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(response_text.find("HTTP/1.1 204 No Content") !=
      std::string::npos);
  }
}
