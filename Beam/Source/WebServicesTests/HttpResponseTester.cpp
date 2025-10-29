#include <sstream>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/HttpResponse.hpp"

using namespace Beam;

TEST_SUITE("HttpResponse") {
  TEST_CASE("default_constructor") {
    auto response = HttpResponse();
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_version() == HttpVersion::version_1_1());
    REQUIRE(response.get_body().get_size() == 0);
    REQUIRE(response.get_header("Content-Length"));
    REQUIRE(*response.get_header("Content-Length") == "0");
    REQUIRE(response.get_header("Connection"));
    REQUIRE(*response.get_header("Connection") == "keep-alive");
  }

  TEST_CASE("status_code_constructor") {
    auto response = HttpResponse(HttpStatusCode::NOT_FOUND);
    REQUIRE(response.get_status_code() == HttpStatusCode::NOT_FOUND);
    REQUIRE(response.get_version() == HttpVersion::version_1_1());
  }

  TEST_CASE("full_constructor") {
    auto headers = std::vector<HttpHeader>();
    headers.emplace_back("Content-Type", "text/html");
    auto cookies = std::vector<Cookie>();
    cookies.emplace_back("session", "abc123");
    auto body = from<SharedBuffer>("test");
    auto response = HttpResponse(HttpVersion::version_1_0(),
      HttpStatusCode::OK, headers, cookies, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_version() == HttpVersion::version_1_0());
    REQUIRE(response.get_headers().size() == 1);
    REQUIRE(response.get_cookies().size() == 1);
    REQUIRE(response.get_body() == "test");
  }

  TEST_CASE("get_set_version") {
    auto response = HttpResponse();
    response.set_version(HttpVersion::version_1_0());
    REQUIRE(response.get_version() == HttpVersion::version_1_0());
  }

  TEST_CASE("get_set_status_code") {
    auto response = HttpResponse();
    response.set_status_code(HttpStatusCode::NOT_FOUND);
    REQUIRE(response.get_status_code() == HttpStatusCode::NOT_FOUND);
  }

  TEST_CASE("set_header_new") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("X-Custom", "value"));
    auto header = response.get_header("X-Custom");
    REQUIRE(header);
    REQUIRE(*header == "value");
  }

  TEST_CASE("set_header_existing") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("Content-Type", "text/html"));
    response.set_header(HttpHeader("Content-Type", "application/json"));
    auto header = response.get_header("Content-Type");
    REQUIRE(header);
    REQUIRE(*header == "application/json");
  }

  TEST_CASE("get_header_not_found") {
    auto response = HttpResponse();
    auto header = response.get_header("NonExistent");
    REQUIRE(!header);
  }

  TEST_CASE("get_headers") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("X-Custom-1", "value1"));
    response.set_header(HttpHeader("X-Custom-2", "value2"));
    auto& headers = response.get_headers();
    REQUIRE(headers.size() >= 2);
  }

  TEST_CASE("multiple_headers") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("Content-Type", "text/html"));
    response.set_header(HttpHeader("Server", "TestServer"));
    response.set_header(HttpHeader("Cache-Control", "no-cache"));
    REQUIRE(response.get_header("Content-Type"));
    REQUIRE(response.get_header("Server"));
    REQUIRE(response.get_header("Cache-Control"));
  }

  TEST_CASE("header_case_sensitive") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("X-Custom", "value"));
    REQUIRE(response.get_header("X-Custom"));
    REQUIRE(!response.get_header("x-custom"));
  }

  TEST_CASE("header_value_with_special_characters") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("X-Special", "value:with:colons"));
    auto header = response.get_header("X-Special");
    REQUIRE(header);
    REQUIRE(*header == "value:with:colons");
  }

  TEST_CASE("set_cookie_new") {
    auto response = HttpResponse();
    auto cookie = Cookie("session", "abc123");
    response.set_cookie(cookie);
    auto retrieved = response.get_cookie("session");
    REQUIRE(retrieved);
    REQUIRE(retrieved->get_value() == "abc123");
  }

  TEST_CASE("set_cookie_existing") {
    auto response = HttpResponse();
    response.set_cookie(Cookie("session", "old"));
    response.set_cookie(Cookie("session", "new"));
    auto cookie = response.get_cookie("session");
    REQUIRE(cookie);
    REQUIRE(cookie->get_value() == "new");
  }

  TEST_CASE("get_cookie_not_found") {
    auto response = HttpResponse();
    auto cookie = response.get_cookie("nonexistent");
    REQUIRE(!cookie);
  }

  TEST_CASE("get_cookies") {
    auto response = HttpResponse();
    response.set_cookie(Cookie("cookie1", "value1"));
    response.set_cookie(Cookie("cookie2", "value2"));
    auto& cookies = response.get_cookies();
    REQUIRE(cookies.size() == 2);
  }

  TEST_CASE("multiple_cookies") {
    auto response = HttpResponse();
    response.set_cookie(Cookie("session", "abc"));
    response.set_cookie(Cookie("token", "xyz"));
    response.set_cookie(Cookie("preference", "dark"));
    REQUIRE(response.get_cookie("session"));
    REQUIRE(response.get_cookie("token"));
    REQUIRE(response.get_cookie("preference"));
  }

  TEST_CASE("cookie_with_attributes") {
    auto response = HttpResponse();
    auto cookie = Cookie("session", "abc123");
    cookie.set_path("/");
    cookie.set_secure(true);
    response.set_cookie(cookie);
    auto retrieved = response.get_cookie("session");
    REQUIRE(retrieved);
    REQUIRE(retrieved->get_path() == "/");
    REQUIRE(retrieved->is_secure());
  }

  TEST_CASE("cookie_name_lookup") {
    auto response = HttpResponse();
    response.set_cookie(Cookie("session_id", "abc"));
    response.set_cookie(Cookie("user_token", "xyz"));
    REQUIRE(response.get_cookie("session_id")->get_value() == "abc");
    REQUIRE(response.get_cookie("user_token")->get_value() == "xyz");
    REQUIRE(!response.get_cookie("nonexistent"));
  }

  TEST_CASE("get_set_body") {
    auto response = HttpResponse();
    auto body = from<SharedBuffer>("hello world");
    response.set_body(body);
    REQUIRE(response.get_body() == "hello world");
  }

  TEST_CASE("set_body_updates_content_length") {
    auto response = HttpResponse();
    auto body = from<SharedBuffer>("test");
    response.set_body(body);
    auto header = response.get_header("Content-Length");
    REQUIRE(header);
    REQUIRE(*header == "4");
  }

  TEST_CASE("large_body") {
    auto response = HttpResponse();
    auto large_data = std::string(10000, 'x');
    auto body = SharedBuffer(large_data.data(), large_data.size());
    response.set_body(body);
    REQUIRE(response.get_body().get_size() == 10000);
    auto header = response.get_header("Content-Length");
    REQUIRE(header);
    REQUIRE(*header == "10000");
  }

  TEST_CASE("body_with_binary_data") {
    auto response = HttpResponse();
    auto data = std::array<char, 4>{'\0', '\1', '\2', '\3'};
    auto body = SharedBuffer(data.data(), data.size());
    response.set_body(body);
    REQUIRE(response.get_body().get_size() == 4);
  }

  TEST_CASE("update_body_multiple_times") {
    auto response = HttpResponse();
    response.set_body(SharedBuffer("first", 5));
    REQUIRE(response.get_body() == "first");
    response.set_body(SharedBuffer("second", 6));
    REQUIRE(response.get_body() == "second");
    auto header = response.get_header("Content-Length");
    REQUIRE(header);
    REQUIRE(*header == "6");
  }

  TEST_CASE("encode_simple_response") {
    auto response = HttpResponse(HttpStatusCode::OK);
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    REQUIRE(buffer.get_size() > 0);
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("HTTP/1.1") != std::string::npos);
    REQUIRE(output.find("200") != std::string::npos);
  }

  TEST_CASE("encode_with_headers") {
    auto response = HttpResponse(HttpStatusCode::OK);
    response.set_header(HttpHeader("Content-Type", "text/html"));
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("Content-Type: text/html") != std::string::npos);
  }

  TEST_CASE("encode_with_cookies") {
    auto response = HttpResponse(HttpStatusCode::OK);
    response.set_cookie(Cookie("session", "abc123"));
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("Set-Cookie:") != std::string::npos);
    REQUIRE(output.find("session") != std::string::npos);
  }

  TEST_CASE("encode_with_body") {
    auto response = HttpResponse(HttpStatusCode::OK);
    auto body = from<SharedBuffer>("test body");
    response.set_body(body);
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("test body") != std::string::npos);
  }

  TEST_CASE("encode_404_status") {
    auto response = HttpResponse(HttpStatusCode::NOT_FOUND);
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("404") != std::string::npos);
  }

  TEST_CASE("encode_500_status") {
    auto response = HttpResponse(HttpStatusCode::INTERNAL_SERVER_ERROR);
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("500") != std::string::npos);
  }

  TEST_CASE("encode_http_1_0") {
    auto response = HttpResponse(HttpStatusCode::OK);
    response.set_version(HttpVersion::version_1_0());
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("HTTP/1.0") != std::string::npos);
  }

  TEST_CASE("encode_complete_format") {
    auto response = HttpResponse(HttpStatusCode::OK);
    response.set_header(HttpHeader("Content-Type", "text/plain"));
    auto body = from<SharedBuffer>("Hello");
    response.set_body(body);
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("HTTP/1.1 200") != std::string::npos);
    REQUIRE(output.find("\r\n") != std::string::npos);
    REQUIRE(output.find("Hello") != std::string::npos);
  }

  TEST_CASE("encode_multiple_headers") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("Header1", "value1"));
    response.set_header(HttpHeader("Header2", "value2"));
    response.set_header(HttpHeader("Header3", "value3"));
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(output.find("Header1: value1") != std::string::npos);
    REQUIRE(output.find("Header2: value2") != std::string::npos);
    REQUIRE(output.find("Header3: value3") != std::string::npos);
  }

  TEST_CASE("encode_multiple_cookies") {
    auto response = HttpResponse();
    response.set_cookie(Cookie("cookie1", "value1"));
    response.set_cookie(Cookie("cookie2", "value2"));
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    auto first_cookie = output.find("Set-Cookie:");
    auto second_cookie = output.find("Set-Cookie:", first_cookie + 1);
    REQUIRE(first_cookie != std::string::npos);
    REQUIRE(second_cookie != std::string::npos);
  }

  TEST_CASE("encode_preserves_header_order") {
    auto response = HttpResponse();
    response.set_header(HttpHeader("A-Header", "1"));
    response.set_header(HttpHeader("B-Header", "2"));
    response.set_header(HttpHeader("C-Header", "3"));
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    auto pos_a = output.find("A-Header");
    auto pos_b = output.find("B-Header");
    auto pos_c = output.find("C-Header");
    REQUIRE(pos_a < pos_b);
    REQUIRE(pos_b < pos_c);
  }

  TEST_CASE("encode_ends_with_double_crlf") {
    auto response = HttpResponse();
    auto buffer = SharedBuffer();
    response.encode(out(buffer));
    auto output = std::string(buffer.get_data(), buffer.get_size());
    auto header_end = output.find("\r\n\r\n");
    REQUIRE(header_end != std::string::npos);
  }

  TEST_CASE("stream_operator") {
    auto response = HttpResponse(HttpStatusCode::OK);
    auto stream = std::stringstream();
    stream << response;
    auto output = stream.str();
    REQUIRE(output.find("HTTP/1.1") != std::string::npos);
    REQUIRE(output.find("200") != std::string::npos);
  }
}
