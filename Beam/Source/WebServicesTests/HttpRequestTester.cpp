#include <sstream>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/HttpRequest.hpp"

using namespace Beam;

TEST_SUITE("HttpRequest") {
  TEST_CASE("construct_get_with_uri") {
    auto request = HttpRequest("http://example.com/path");
    REQUIRE(request.get_method() == HttpMethod::GET);
    REQUIRE(request.get_uri().get_hostname() == "example.com");
    REQUIRE(request.get_uri().get_path() == "/path");
    REQUIRE(request.get_version() == HttpVersion::version_1_1());
    REQUIRE(request.get_special_headers().m_host == "example.com");
    REQUIRE(request.get_special_headers().m_content_length == 0);
    REQUIRE(request.get_special_headers().m_connection ==
      ConnectionHeader::KEEP_ALIVE);
  }

  TEST_CASE("construct_with_method_and_uri") {
    auto request = HttpRequest(HttpMethod::POST, "http://example.com");
    REQUIRE(request.get_method() == HttpMethod::POST);
    REQUIRE(request.get_version() == HttpVersion::version_1_1());
  }

  TEST_CASE("construct_with_body") {
    auto body = SharedBuffer("test body", 9);
    auto request =
      HttpRequest(HttpMethod::POST, "http://example.com", std::move(body));
    REQUIRE(request.get_method() == HttpMethod::POST);
    REQUIRE(request.get_body().get_size() == 9);
    REQUIRE(request.get_special_headers().m_content_length == 9);
  }

  TEST_CASE("construct_with_version") {
    auto request = HttpRequest(
      HttpVersion::version_1_0(), HttpMethod::GET, "http://example.com");
    REQUIRE(request.get_version() == HttpVersion::version_1_0());
    REQUIRE(
      request.get_special_headers().m_connection == ConnectionHeader::CLOSE);
  }

  TEST_CASE("construct_complete") {
    auto headers = std::vector<HttpHeader>();
    headers.push_back(HttpHeader("X-Custom", "value"));
    auto special_headers = SpecialHeaders();
    special_headers.m_host = "custom.com";
    auto cookies = std::vector<Cookie>();
    cookies.push_back(Cookie("session", "abc123"));
    auto body = SharedBuffer("data", 4);
    auto request = HttpRequest(HttpVersion::version_1_1(), HttpMethod::POST,
      "http://example.com", std::move(headers), special_headers,
      std::move(cookies), std::move(body));
    REQUIRE(request.get_headers().size() == 1);
    REQUIRE(request.get_headers()[0].get_name() == "X-Custom");
    REQUIRE(request.get_special_headers().m_host == "custom.com");
    REQUIRE(request.get_cookies().size() == 1);
    REQUIRE(request.get_body().get_size() == 4);
  }

  TEST_CASE("get_header") {
    auto request = HttpRequest("http://example.com");
    request.add(HttpHeader("Accept", "application/json"));
    auto header = request.get_header("Accept");
    REQUIRE(header);
    REQUIRE(*header == "application/json");
    auto missing = request.get_header("Missing");
    REQUIRE(!missing);
  }

  TEST_CASE("get_special_header_host") {
    auto request = HttpRequest("http://example.com");
    auto host = request.get_header("Host");
    REQUIRE(host);
    REQUIRE(*host == "example.com");
  }

  TEST_CASE("get_special_header_content_length") {
    auto body = SharedBuffer("test", 4);
    auto request =
      HttpRequest(HttpMethod::POST, "http://example.com", std::move(body));
    auto length = request.get_header("Content-Length");
    REQUIRE(length);
    REQUIRE(*length == "4");
  }

  TEST_CASE("get_special_header_connection") {
    auto request = HttpRequest("http://example.com");
    auto connection = request.get_header("Connection");
    REQUIRE(connection);
    REQUIRE(*connection == "keep-alive");
  }

  TEST_CASE("add_header") {
    auto request = HttpRequest("http://example.com");
    REQUIRE(request.get_headers().empty());
    request.add(HttpHeader("User-Agent", "TestClient"));
    REQUIRE(request.get_headers().size() == 1);
    REQUIRE(request.get_headers()[0].get_name() == "User-Agent");
    REQUIRE(request.get_headers()[0].get_value() == "TestClient");
  }

  TEST_CASE("add_special_header_host") {
    auto request = HttpRequest("http://example.com");
    request.add(HttpHeader("Host", "override.com"));
    REQUIRE(request.get_special_headers().m_host == "override.com");
    REQUIRE(request.get_headers().empty());
  }

  TEST_CASE("add_special_header_content_length") {
    auto request = HttpRequest("http://example.com");
    request.add(HttpHeader("Content-Length", "100"));
    REQUIRE(request.get_special_headers().m_content_length == 100);
  }

  TEST_CASE("add_special_header_connection") {
    auto request = HttpRequest("http://example.com");
    request.add(HttpHeader("Connection", "close"));
    REQUIRE(request.get_special_headers().m_connection ==
      ConnectionHeader::CLOSE);
    request.add(HttpHeader("Connection", "Upgrade"));
    REQUIRE(request.get_special_headers().m_connection ==
      ConnectionHeader::UPGRADE);
  }

  TEST_CASE("add_invalid_connection_header") {
    auto request = HttpRequest("http://example.com");
    REQUIRE_THROWS_AS(request.add(HttpHeader("Connection", "invalid")),
      std::runtime_error);
  }

  TEST_CASE("get_cookie") {
    auto request = HttpRequest("http://example.com");
    request.add(Cookie("session", "abc123"));
    auto cookie = request.get_cookie("session");
    REQUIRE(cookie);
    REQUIRE(cookie->get_name() == "session");
    REQUIRE(cookie->get_value() == "abc123");
    auto missing = request.get_cookie("missing");
    REQUIRE(!missing);
  }

  TEST_CASE("add_cookie") {
    auto request = HttpRequest("http://example.com");
    REQUIRE(request.get_cookies().empty());
    request.add(Cookie("token", "xyz789"));
    REQUIRE(request.get_cookies().size() == 1);
    REQUIRE(request.get_cookies()[0].get_name() == "token");
  }

  TEST_CASE("host_with_non_default_port") {
    auto request = HttpRequest("http://example.com:8080");
    REQUIRE(request.get_special_headers().m_host == "example.com:8080");
  }

  TEST_CASE("host_with_default_http_port") {
    auto request = HttpRequest("http://example.com:80");
    REQUIRE(request.get_special_headers().m_host == "example.com");
  }

  TEST_CASE("host_with_default_https_port") {
    auto request = HttpRequest("https://example.com:443");
    REQUIRE(request.get_special_headers().m_host == "example.com");
  }

  TEST_CASE("post_with_query_string") {
    auto request =
      HttpRequest(HttpMethod::POST, "http://example.com?key=value");
    REQUIRE(request.get_body().get_size() == 9);
    REQUIRE(request.get_special_headers().m_content_length == 9);
    auto content_type = request.get_header("Content-Type");
    REQUIRE(content_type);
    REQUIRE(*content_type == "application/x-www-form-urlencoded");
  }

  TEST_CASE("stream_basic") {
    auto request = HttpRequest("http://example.com/path");
    auto ss = std::stringstream();
    ss << request;
    auto output = ss.str();
    REQUIRE(output.find("GET /path HTTP/1.1\r\n") != std::string::npos);
    REQUIRE(output.find("Host: example.com\r\n") != std::string::npos);
    REQUIRE(output.find("Content-Length: 0\r\n") != std::string::npos);
    REQUIRE(output.find("Connection: keep-alive\r\n") != std::string::npos);
  }

  TEST_CASE("stream_with_query") {
    auto request = HttpRequest("http://example.com/search?q=test");
    auto ss = std::stringstream();
    ss << request;
    auto output = ss.str();
    REQUIRE(output.find("GET /search?q=test HTTP/1.1\r\n") !=
      std::string::npos);
  }

  TEST_CASE("stream_root_path") {
    auto request = HttpRequest("http://example.com");
    auto ss = std::stringstream();
    ss << request;
    auto output = ss.str();
    REQUIRE(output.find("GET / HTTP/1.1\r\n") != std::string::npos);
  }

  TEST_CASE("stream_with_headers") {
    auto request = HttpRequest("http://example.com");
    request.add(HttpHeader("Accept", "application/json"));
    request.add(HttpHeader("User-Agent", "TestClient"));
    auto ss = std::stringstream();
    ss << request;
    auto output = ss.str();
    REQUIRE(output.find("Accept: application/json\r\n") != std::string::npos);
    REQUIRE(output.find("User-Agent: TestClient\r\n") != std::string::npos);
  }

  TEST_CASE("stream_with_cookies") {
    auto request = HttpRequest("http://example.com");
    request.add(Cookie("session", "abc"));
    request.add(Cookie("token", "xyz"));
    auto ss = std::stringstream();
    ss << request;
    auto output = ss.str();
    REQUIRE(output.find("Cookie: session=abc; token=xyz\r\n") !=
      std::string::npos);
  }

  TEST_CASE("stream_post_method") {
    auto body = SharedBuffer("data", 4);
    auto request =
      HttpRequest(HttpMethod::POST, "http://example.com", std::move(body));
    auto ss = std::stringstream();
    ss << request;
    auto output = ss.str();
    REQUIRE(output.find("POST / HTTP/1.1\r\n") != std::string::npos);
    REQUIRE(output.find("Content-Length: 4\r\n") != std::string::npos);
  }

  TEST_CASE("stream_connection_header") {
    auto ss1 = std::stringstream();
    ss1 << ConnectionHeader::CLOSE;
    REQUIRE(ss1.str() == "close");
    auto ss2 = std::stringstream();
    ss2 << ConnectionHeader::KEEP_ALIVE;
    REQUIRE(ss2.str() == "keep-alive");
    auto ss3 = std::stringstream();
    ss3 << ConnectionHeader::UPGRADE;
    REQUIRE(ss3.str() == "Upgrade");
  }

  TEST_CASE("special_headers_default") {
    auto headers = SpecialHeaders();
    REQUIRE(headers.m_content_length == 0);
    REQUIRE(headers.m_connection == ConnectionHeader::KEEP_ALIVE);
  }

  TEST_CASE("special_headers_version_1_1") {
    auto headers = SpecialHeaders(HttpVersion::version_1_1());
    REQUIRE(headers.m_connection == ConnectionHeader::KEEP_ALIVE);
  }

  TEST_CASE("special_headers_version_1_0") {
    auto headers = SpecialHeaders(HttpVersion::version_1_0());
    REQUIRE(headers.m_connection == ConnectionHeader::CLOSE);
  }
}
