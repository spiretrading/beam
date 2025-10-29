#include <sstream>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/HttpRequestParser.hpp"

using namespace Beam;

TEST_SUITE("HttpRequestParser") {
  TEST_CASE("parse_simple_get") {
    auto parser = HttpRequestParser();
    parser.feed("GET /path HTTP/1.1\r\n\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_method() == HttpMethod::GET);
    REQUIRE(request->get_uri().get_path() == "/path");
    REQUIRE(request->get_version() == HttpVersion::version_1_1());
  }

  TEST_CASE("parse_get_with_headers") {
    auto parser = HttpRequestParser();
    parser.feed("GET /test HTTP/1.1\r\n");
    parser.feed("Host: example.com\r\n");
    parser.feed("User-Agent: TestClient\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_headers().size() == 1);
    REQUIRE(request->get_headers()[0].get_name() == "User-Agent");
    REQUIRE(request->get_headers()[0].get_value() == "TestClient");
  }

  TEST_CASE("parse_post_with_body") {
    auto parser = HttpRequestParser();
    parser.feed("POST /api HTTP/1.1\r\n");
    parser.feed("Content-Length: 4\r\n");
    parser.feed("\r\ntest");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_method() == HttpMethod::POST);
    REQUIRE(request->get_body().get_size() == 4);
    REQUIRE(request->get_body() == "test");
  }

  TEST_CASE("parse_with_cookies") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Cookie: session=abc123; token=xyz789\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_cookies().size() == 2);
    REQUIRE(request->get_cookies()[0].get_name() == "session");
    REQUIRE(request->get_cookies()[0].get_value() == "abc123");
    REQUIRE(request->get_cookies()[1].get_name() == "token");
    REQUIRE(request->get_cookies()[1].get_value() == "xyz789");
  }

  TEST_CASE("parse_http_1_0") {
    auto parser = HttpRequestParser();
    parser.feed("GET /old HTTP/1.0\r\n\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_version() == HttpVersion::version_1_0());
    REQUIRE(
      request->get_special_headers().m_connection == ConnectionHeader::CLOSE);
  }

  TEST_CASE("parse_connection_header") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Connection: keep-alive\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_special_headers().m_connection ==
      ConnectionHeader::KEEP_ALIVE);
  }

  TEST_CASE("parse_multiple_requests") {
    auto parser = HttpRequestParser();
    parser.feed("GET /first HTTP/1.1\r\n\r\n");
    parser.feed("GET /second HTTP/1.1\r\n\r\n");
    auto first = parser.get_next_request();
    REQUIRE(first);
    REQUIRE(first->get_uri().get_path() == "/first");
    auto second = parser.get_next_request();
    REQUIRE(second);
    REQUIRE(second->get_uri().get_path() == "/second");
  }

  TEST_CASE("parse_incremental_feeding") {
    auto parser = HttpRequestParser();
    parser.feed("GET /incre");
    parser.feed("mental HTTP/1.1\r");
    parser.feed("\n\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_uri().get_path() == "/incremental");
  }

  TEST_CASE("parse_empty_before_request") {
    auto parser = HttpRequestParser();
    auto request = parser.get_next_request();
    REQUIRE(!request);
  }

  TEST_CASE("parse_incomplete_request") {
    auto parser = HttpRequestParser();
    parser.feed("GET /incomplete HTTP/1.1\r\n");
    auto request = parser.get_next_request();
    REQUIRE(!request);
  }

  TEST_CASE("parse_body_with_content_length") {
    auto parser = HttpRequestParser();
    parser.feed("POST /data HTTP/1.1\r\n");
    parser.feed("Content-Length: 11\r\n");
    parser.feed("\r\nhello world");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body().get_size() == 11);
    REQUIRE(request->get_body() == "hello world");
  }

  TEST_CASE("parse_cookie_without_equals") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Cookie: standalone\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_cookies().size() == 1);
    REQUIRE(request->get_cookies()[0].get_name().empty());
    REQUIRE(request->get_cookies()[0].get_value() == "standalone");
  }

  TEST_CASE("parse_with_query_string") {
    auto parser = HttpRequestParser();
    parser.feed("GET /search?q=test&page=1 HTTP/1.1\r\n\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_uri().get_path() == "/search");
    REQUIRE(request->get_uri().get_query() == "q=test&page=1");
  }

  TEST_CASE("parse_header_with_colon_in_value") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("X-Custom: value:with:colons\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_headers().size() == 1);
    REQUIRE(request->get_headers()[0].get_value() == "value:with:colons");
  }

  TEST_CASE("parse_upgrade_connection") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Connection: Upgrade\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_special_headers().m_connection ==
      ConnectionHeader::UPGRADE);
  }

  TEST_CASE("parse_body_split_across_feeds") {
    auto parser = HttpRequestParser();
    parser.feed("POST /split HTTP/1.1\r\n");
    parser.feed("Content-Length: 8\r\n");
    parser.feed("\r\nabcd");
    parser.feed("efgh");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body().get_size() == 8);
    REQUIRE(request->get_body() == "abcdefgh");
  }

  TEST_CASE("invalid_method") {
    auto parser = HttpRequestParser();
    parser.feed("INVALID /path HTTP/1.1\r\n\r\n");
    REQUIRE_THROWS_AS(parser.get_next_request(), InvalidHttpRequestException);
  }

  TEST_CASE("invalid_http_version") {
    auto parser = HttpRequestParser();
    parser.feed("GET /path HTTP/2.0\r\n\r\n");
    REQUIRE_THROWS_AS(parser.get_next_request(), InvalidHttpRequestException);
  }

  TEST_CASE("malformed_uri") {
    auto parser = HttpRequestParser();
    parser.feed("GET not_a_valid_uri!@#$ HTTP/1.1\r\n\r\n");
    REQUIRE_THROWS_AS(parser.get_next_request(), InvalidHttpRequestException);
  }

  TEST_CASE("missing_space_after_method") {
    auto parser = HttpRequestParser();
    parser.feed("GET/path HTTP/1.1\r\n\r\n");
    REQUIRE_THROWS_AS(parser.get_next_request(), InvalidHttpRequestException);
  }

  TEST_CASE("header_without_space_after_colon") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("BadHeader:value\r\n");
    parser.feed("\r\n");
    REQUIRE_THROWS_AS(parser.get_next_request(), InvalidHttpRequestException);
  }

  TEST_CASE("parse_zero_length_body") {
    auto parser = HttpRequestParser();
    parser.feed("POST /empty HTTP/1.1\r\n");
    parser.feed("Content-Length: 0\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body().get_size() == 0);
  }

  TEST_CASE("parse_connection_close") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Connection: close\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(
      request->get_special_headers().m_connection == ConnectionHeader::CLOSE);
  }

  TEST_CASE("parse_multiple_cookies_single_header") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Cookie: a=1; b=2; c=3\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_cookies().size() == 3);
    REQUIRE(request->get_cookies()[0].get_name() == "a");
    REQUIRE(request->get_cookies()[1].get_name() == "b");
    REQUIRE(request->get_cookies()[2].get_name() == "c");
  }

  TEST_CASE("incremental_feed_request_line_split") {
    auto parser = HttpRequestParser();
    parser.feed("GET /");
    parser.feed("path HT");
    parser.feed("TP/1.1\r");
    parser.feed("\n\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_uri().get_path() == "/path");
  }

  TEST_CASE("incremental_feed_header_name_split") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Con");
    parser.feed("tent-Len");
    parser.feed("gth: 4\r\n");
    parser.feed("\r\ntest");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body() == "test");
  }

  TEST_CASE("incremental_feed_header_value_split") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("User-Agent: Mo");
    parser.feed("zilla/5.0\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_headers().size() == 1);
    REQUIRE(request->get_headers()[0].get_value() == "Mozilla/5.0");
  }

  TEST_CASE("incremental_feed_crlf_split") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r");
    parser.feed("\nHost: test.com\r");
    parser.feed("\n\r");
    parser.feed("\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_special_headers().m_host == "test.com");
  }

  TEST_CASE("incremental_feed_body_one_byte_at_a_time") {
    auto parser = HttpRequestParser();
    parser.feed("POST / HTTP/1.1\r\n");
    parser.feed("Content-Length: 5\r\n");
    parser.feed("\r\n");
    parser.feed("h");
    parser.feed("e");
    parser.feed("l");
    parser.feed("l");
    parser.feed("o");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body() == "hello");
  }

  TEST_CASE("incremental_feed_cookie_split") {
    auto parser = HttpRequestParser();
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("Cookie: ses");
    parser.feed("sion=abc");
    parser.feed("123; tok");
    parser.feed("en=xyz\r\n");
    parser.feed("\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_cookies().size() == 2);
    REQUIRE(request->get_cookies()[0].get_value() == "abc123");
    REQUIRE(request->get_cookies()[1].get_value() == "xyz");
  }

  TEST_CASE("incremental_feed_empty_line_split") {
    auto parser = HttpRequestParser();
    parser.feed("POST / HTTP/1.1\r\n");
    parser.feed("Content-Length: 3\r\n");
    parser.feed("\r");
    parser.feed("\n");
    parser.feed("abc");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body() == "abc");
  }

  TEST_CASE("incremental_feed_multiple_requests_interleaved") {
    auto parser = HttpRequestParser();
    parser.feed("GET /first HTTP/1.1\r\n\r\nGET /sec");
    auto first = parser.get_next_request();
    REQUIRE(first);
    REQUIRE(first->get_uri().get_path() == "/first");
    parser.feed("ond HTTP/1.1\r\n\r\n");
    auto second = parser.get_next_request();
    REQUIRE(second);
    REQUIRE(second->get_uri().get_path() == "/second");
  }

  TEST_CASE("incremental_feed_post_then_get") {
    auto parser = HttpRequestParser();
    parser.feed("POST / HTTP/1.1\r\n");
    parser.feed("Content-Length: 2\r\n");
    parser.feed("\r\nAB");
    parser.feed("GET /next HTTP/1.1\r\n\r\n");
    auto first = parser.get_next_request();
    REQUIRE(first);
    REQUIRE(first->get_method() == HttpMethod::POST);
    REQUIRE(first->get_body() == "AB");
    auto second = parser.get_next_request();
    REQUIRE(second);
    REQUIRE(second->get_method() == HttpMethod::GET);
    REQUIRE(second->get_uri().get_path() == "/next");
  }

  TEST_CASE("incremental_feed_very_long_header_value") {
    auto parser = HttpRequestParser();
    auto long_value = std::string(1000, 'x');
    parser.feed("GET / HTTP/1.1\r\n");
    parser.feed("X-Long: ");
    for(auto i = std::size_t(0); i < long_value.size(); i += 10) {
      parser.feed(long_value.substr(i, 10));
    }
    parser.feed("\r\n\r\n");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_headers().size() == 1);
    REQUIRE(request->get_headers()[0].get_value() == long_value);
  }

  TEST_CASE("incremental_feed_body_boundary_split") {
    auto parser = HttpRequestParser();
    parser.feed("POST / HTTP/1.1\r\n");
    parser.feed("Content-Length: 10\r\n");
    parser.feed("\r\nabc");
    REQUIRE(!parser.get_next_request());
    parser.feed("def");
    REQUIRE(!parser.get_next_request());
    parser.feed("ghij");
    auto request = parser.get_next_request();
    REQUIRE(request);
    REQUIRE(request->get_body() == "abcdefghij");
  }
}
