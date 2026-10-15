#include <boost/date_time/posix_time/time_parsers.hpp>
#include <doctest/doctest.h>
#include "Beam/Utilities/ToString.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/TransferEncoding.hpp"

using namespace Beam;
using namespace boost::posix_time;

namespace {
  void test_format(const auto& value, const std::string& expected) {
    REQUIRE(std::format("{}", value) == expected);
    REQUIRE(to_string(value) == expected);
    REQUIRE(std::format("{:*>{}}", value, expected.size() + 2) ==
      std::format("{:*>{}}", expected, expected.size() + 2));
    REQUIRE(std::format("{:.3}", value) == expected.substr(0, 3));
  }
}

TEST_SUITE("WebFormat") {
  TEST_CASE("format") {
    SUBCASE("metadata") {
      test_format(EmailAddress("alice@example.com"), "alice@example.com");
      test_format(EmailAddress("alice@example.com", "Alice"),
        "Alice <alice@example.com>");
      test_format(HttpVersion::version_1_0(), "HTTP/1.0");
      test_format(HttpVersion::version_1_1(), "HTTP/1.1");
      test_format(HttpHeader("Content-Type", "text/plain"),
        "Content-Type: text/plain");
      test_format(HttpHeader("X-Empty", ""), "X-Empty: ");
      test_format(HttpMethod::GET, "GET");
      test_format(HttpMethod(HttpMethod::POST), "POST");
      test_format(ConnectionHeader::CLOSE, "close");
      test_format(ConnectionHeader::KEEP_ALIVE, "keep-alive");
      test_format(ConnectionHeader::UPGRADE, "Upgrade");
      test_format(static_cast<ConnectionHeader>(99), "Upgrade");
      test_format(TransferEncoding::NONE, "none");
      test_format(TransferEncoding::CHUNKED, "chunked");
      test_format(TransferEncoding::COMPRESS, "compress");
      test_format(TransferEncoding::DEFLATE, "deflate");
      test_format(TransferEncoding::GZIP, "gzip");
      test_format(TransferEncoding::IDENTITY, "identity");
      test_format(static_cast<TransferEncoding>(99), "none");
    }
    SUBCASE("uri") {
      test_format(Uri(), "");
      test_format(Uri("https://example.com:443/path?key=value#section"),
        "https://example.com/path?key=value#section");
      test_format(Uri("http://alice:secret@example.com:8080/path"),
        "http://alice:secret@example.com:8080/path");
      test_format(Uri("/path?key=value"), "/path?key=value");
    }
    SUBCASE("cookie") {
      auto cookie = Cookie("session", "abc");
      test_format(cookie, "session=abc; Path=/");
      cookie.set_domain("example.com");
      cookie.set_path("/app");
      cookie.set_expiration(time_from_string("2026-10-15 09:30:00"));
      cookie.set_secure(true);
      cookie.set_http_only(true);
      test_format(cookie,
        "session=abc; Domain=example.com; Path=/app; "
        "Expires=Thu, 15 Oct 2026 09:30:00 GMT; Secure; HttpOnly");
    }
    SUBCASE("request") {
      auto request = HttpRequest(Uri("http://example.com/path?key=value"));
      request.add(HttpHeader("X-Test", "value"));
      request.add(Cookie("session", "abc"));
      test_format(request,
        "GET /path?key=value HTTP/1.1\r\n"
        "X-Test: value\r\nCookie: session=abc\r\n"
        "Host: example.com\r\nContent-Length: 0\r\n"
        "Connection: keep-alive\r\n\r\n");
      auto post = HttpRequest(HttpMethod::POST, Uri("http://example.com"),
        SharedBuffer("a\0b", 3));
      auto expected = std::string(
        "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 3\r\n"
        "Connection: keep-alive\r\n\r\n") + std::string("a\0b", 3);
      test_format(post, expected);
      auto encoded = SharedBuffer();
      post.encode(out(encoded));
      REQUIRE(std::format("{}", post) == std::format("{}", encoded));
    }
    SUBCASE("response") {
      auto response = HttpResponse(HttpStatusCode::OK);
      response.set_header(HttpHeader("Content-Type", "text/plain"));
      response.set_cookie(Cookie("session", "abc"));
      response.set_body(SharedBuffer("a\0b", 3));
      auto expected = std::string(
        "HTTP/1.1 200 OK\r\nContent-Length: 3\r\nConnection: keep-alive\r\n"
        "Content-Type: text/plain\r\nSet-Cookie: session=abc; Path=/\r\n\r\n") +
          std::string("a\0b", 3);
      test_format(response, expected);
      auto encoded = SharedBuffer();
      response.encode(out(encoded));
      REQUIRE(std::format("{}", response) == std::format("{}", encoded));
    }
    SUBCASE("email") {
      auto email = Email("alice@example.com", "bob@example.com");
      email.set_date(time_from_string("2026-10-15 09:30:00"));
      email.set_subject("Hello");
      email.add_body(".first\r\n.second");
      test_format(email,
        "From: alice@example.com\r\nDate: Thu, 15 Oct 2026 09:30:00 +0000\r\n"
        "To: bob@example.com\r\nSubject: Hello\r\nMime-Version: 1.0\r\n"
        "Content-Type: text/plain\r\n\r\n..first\r\n..second\r\n.\r\n");
    }
  }
}
