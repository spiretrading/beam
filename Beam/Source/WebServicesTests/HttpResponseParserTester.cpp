#include <sstream>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/HttpResponseParser.hpp"

using namespace Beam;

TEST_SUITE("HttpResponseParser") {
  TEST_CASE("parse_simple_200") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_status_code() == HttpStatusCode::OK);
    REQUIRE(response->get_version() == HttpVersion::version_1_1());
  }

  TEST_CASE("parse_with_headers") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Type: text/html\r\n");
    parser.feed("Content-Length: 0\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_header("Content-Type"));
    REQUIRE(*response->get_header("Content-Type") == "text/html");
  }

  TEST_CASE("parse_with_body") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 11\r\n");
    parser.feed("\r\nhello world");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "hello world");
  }

  TEST_CASE("parse_with_cookies") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: session=abc123; path=/; secure\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 1);
    REQUIRE(response->get_cookies()[0].get_name() == "session");
    REQUIRE(response->get_cookies()[0].get_value() == "abc123");
    REQUIRE(response->get_cookies()[0].is_secure());
  }

  TEST_CASE("parse_http_1_0") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.0 200 OK\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_version() == HttpVersion::version_1_0());
  }

  TEST_CASE("parse_404_status") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 404 Not Found\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_status_code() == HttpStatusCode::NOT_FOUND);
  }

  TEST_CASE("parse_500_status") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 500 Internal Server Error\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(
      response->get_status_code() == HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("parse_multiple_responses") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n\r\n");
    parser.feed("HTTP/1.1 404 Not Found\r\n\r\n");
    auto first = parser.get_next_response();
    REQUIRE(first);
    REQUIRE(first->get_status_code() == HttpStatusCode::OK);
    auto second = parser.get_next_response();
    REQUIRE(second);
    REQUIRE(second->get_status_code() == HttpStatusCode::NOT_FOUND);
  }

  TEST_CASE("parse_empty_before_response") {
    auto parser = HttpResponseParser();
    auto response = parser.get_next_response();
    REQUIRE(!response);
  }

  TEST_CASE("parse_incomplete_response") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    auto response = parser.get_next_response();
    REQUIRE(!response);
  }

  TEST_CASE("parse_body_with_content_length") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 4\r\n");
    parser.feed("\r\ntest");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body().get_size() == 4);
    REQUIRE(response->get_body() == "test");
  }

  TEST_CASE("parse_cookie_with_domain") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: id=123; domain=example.com\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 1);
    REQUIRE(response->get_cookies()[0].get_domain() == "example.com");
  }

  TEST_CASE("parse_cookie_with_path") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: id=123; path=/admin\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 1);
    REQUIRE(response->get_cookies()[0].get_path() == "/admin");
  }

  TEST_CASE("parse_cookie_http_only") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: id=123; HttpOnly\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 1);
    REQUIRE(response->get_cookies()[0].is_http_only());
  }

  TEST_CASE("parse_multiple_headers") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Type: text/html\r\n");
    parser.feed("Server: TestServer\r\n");
    parser.feed("Cache-Control: no-cache\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_headers().size() == 3);
  }

  TEST_CASE("parse_header_with_colon_in_value") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("X-Custom: value:with:colons\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_header("X-Custom"));
    REQUIRE(*response->get_header("X-Custom") == "value:with:colons");
  }

  TEST_CASE("parse_zero_length_body") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 0\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body().get_size() == 0);
  }

  TEST_CASE("parse_chunked_encoding") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\n");
    parser.feed("5\r\nhello\r\n");
    parser.feed("6\r\n world\r\n");
    parser.feed("0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "hello world");
  }

  TEST_CASE("incremental_feed_status_line_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 ");
    parser.feed("200 OK\r");
    parser.feed("\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_status_code() == HttpStatusCode::OK);
  }

  TEST_CASE("incremental_feed_header_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-");
    parser.feed("Type: text/");
    parser.feed("html\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_header("Content-Type"));
    REQUIRE(*response->get_header("Content-Type") == "text/html");
  }

  TEST_CASE("incremental_feed_body_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 10\r\n");
    parser.feed("\r\nabc");
    parser.feed("def");
    parser.feed("ghij");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "abcdefghij");
  }

  TEST_CASE("incremental_feed_crlf_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r");
    parser.feed("\nContent-Type: text/html\r");
    parser.feed("\n\r");
    parser.feed("\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_header("Content-Type"));
  }

  TEST_CASE("incremental_feed_body_one_byte_at_a_time") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 5\r\n");
    parser.feed("\r\n");
    parser.feed("h");
    parser.feed("e");
    parser.feed("l");
    parser.feed("l");
    parser.feed("o");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "hello");
  }

  TEST_CASE("incremental_feed_multiple_responses") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n\r\nHTTP/1.1 ");
    auto first = parser.get_next_response();
    REQUIRE(first);
    REQUIRE(first->get_status_code() == HttpStatusCode::OK);
    parser.feed("404 Not Found\r\n\r\n");
    auto second = parser.get_next_response();
    REQUIRE(second);
    REQUIRE(second->get_status_code() == HttpStatusCode::NOT_FOUND);
  }

  TEST_CASE("incremental_feed_with_body_then_next_response") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 4\r\n");
    parser.feed("\r\ntestHTTP/1.1 404 Not Found\r\n\r\n");
    auto first = parser.get_next_response();
    REQUIRE(first);
    REQUIRE(first->get_status_code() == HttpStatusCode::OK);
    REQUIRE(first->get_body() == "test");
    auto second = parser.get_next_response();
    REQUIRE(second);
    REQUIRE(second->get_status_code() == HttpStatusCode::NOT_FOUND);
  }

  TEST_CASE("incremental_feed_very_long_header_value") {
    auto parser = HttpResponseParser();
    auto long_value = std::string(1000, 'x');
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("X-Long: ");
    for(auto i = std::size_t(0); i < long_value.size(); i += 10) {
      parser.feed(long_value.substr(i, 10));
    }
    parser.feed("\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_header("X-Long"));
    REQUIRE(*response->get_header("X-Long") == long_value);
  }

  TEST_CASE("incremental_feed_chunked_encoding") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\n");
    parser.feed("5\r\n");
    parser.feed("hel");
    parser.feed("lo\r\n");
    parser.feed("6\r\n wo");
    parser.feed("rld\r\n0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "hello world");
  }

  TEST_CASE("incremental_feed_chunked_size_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\n");
    parser.feed("1");
    parser.feed("0\r\n");
    parser.feed("0123456789012345\r\n");
    parser.feed("0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body().get_size() == 16);
  }

  TEST_CASE("incremental_feed_empty_line_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 3\r\n");
    parser.feed("\r");
    parser.feed("\n");
    parser.feed("abc");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "abc");
  }

  TEST_CASE("incremental_feed_cookie_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: ses");
    parser.feed("sion=abc");
    parser.feed("123; path=/\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 1);
    REQUIRE(response->get_cookies()[0].get_value() == "abc123");
  }

  TEST_CASE("incremental_feed_body_boundary_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Content-Length: 10\r\n");
    parser.feed("\r\nabc");
    REQUIRE(!parser.get_next_response());
    parser.feed("def");
    REQUIRE(!parser.get_next_response());
    parser.feed("ghij");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "abcdefghij");
  }

  TEST_CASE("incremental_feed_status_code_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 20");
    parser.feed("0 OK\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_status_code() == HttpStatusCode::OK);
  }

  TEST_CASE("invalid_http_version") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/2.0 200 OK\r\n\r\n");
    REQUIRE_THROWS_AS(
      parser.get_next_response(), InvalidHttpResponseException);
  }

  TEST_CASE("missing_header_space_after_colon") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("BadHeader:value\r\n");
    parser.feed("\r\n");
    REQUIRE_THROWS_AS(
      parser.get_next_response(), InvalidHttpResponseException);
  }

  TEST_CASE("malformed_status_line") {
    auto parser = HttpResponseParser();
    parser.feed("INVALID\r\n\r\n");
    REQUIRE_THROWS_AS(
      parser.get_next_response(), InvalidHttpResponseException);
  }

  TEST_CASE("parse_multiple_set_cookies") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: session=abc; path=/\r\n");
    parser.feed("Set-Cookie: token=xyz; secure\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 2);
    REQUIRE(response->get_cookies()[0].get_name() == "session");
    REQUIRE(response->get_cookies()[1].get_name() == "token");
  }

  TEST_CASE("parse_cookie_without_equals") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Set-Cookie: standalone\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_cookies().size() == 1);
    REQUIRE(response->get_cookies()[0].get_name().empty());
    REQUIRE(response->get_cookies()[0].get_value() == "standalone");
  }

  TEST_CASE("incremental_chunked_multiple_chunks") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\n");
    parser.feed("3\r\nabc\r\n");
    parser.feed("3\r\ndef\r\n");
    parser.feed("3\r\nghi\r\n");
    parser.feed("0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "abcdefghi");
  }

  TEST_CASE("incremental_chunked_with_large_size") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\n");
    auto data = std::string(256, 'x');
    parser.feed("100\r\n");
    parser.feed(data);
    parser.feed("\r\n0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body().get_size() == 256);
  }

  TEST_CASE("parse_no_body_no_content_length") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body().get_size() == 0);
  }

  TEST_CASE("parse_response_with_reason_phrase") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 404 Not Found\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_status_code() == HttpStatusCode::NOT_FOUND);
  }

  TEST_CASE("incremental_feed_header_continuation") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Long-Header: ");
    parser.feed("part1 ");
    parser.feed("part2 ");
    parser.feed("part3\r\n");
    parser.feed("\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_header("Long-Header"));
    REQUIRE(*response->get_header("Long-Header") == "part1 part2 part3");
  }

  TEST_CASE("incremental_feed_chunked_across_boundaries") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\n5");
    parser.feed("\r");
    parser.feed("\nhello\r\n0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "hello");
  }

  TEST_CASE("parse_chunked_with_single_chunk") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.1 200 OK\r\n");
    parser.feed("Transfer-Encoding: chunked\r\n");
    parser.feed("\r\nA\r\n0123456789\r\n0\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_body() == "0123456789");
  }

  TEST_CASE("incremental_version_number_split") {
    auto parser = HttpResponseParser();
    parser.feed("HTTP/1.");
    parser.feed("1 200 OK\r\n\r\n");
    auto response = parser.get_next_response();
    REQUIRE(response);
    REQUIRE(response->get_version() == HttpVersion::version_1_1());
  }
}
