#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/Cookie.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("Cookie") {
  TEST_CASE("construct_empty") {
    auto cookie = Cookie();
    REQUIRE(cookie.get_name().empty());
    REQUIRE(cookie.get_value().empty());
    REQUIRE(cookie.get_domain().empty());
    REQUIRE(cookie.get_path() == "/");
    REQUIRE(cookie.get_expiration().is_not_a_date_time());
    REQUIRE(!cookie.is_secure());
    REQUIRE(!cookie.is_http_only());
  }

  TEST_CASE("construct_with_name_and_value") {
    auto cookie = Cookie("session_id", "abc123");
    REQUIRE(cookie.get_name() == "session_id");
    REQUIRE(cookie.get_value() == "abc123");
    REQUIRE(cookie.get_domain().empty());
    REQUIRE(cookie.get_path() == "/");
    REQUIRE(cookie.get_expiration().is_not_a_date_time());
    REQUIRE(!cookie.is_secure());
    REQUIRE(!cookie.is_http_only());
  }

  TEST_CASE("setters") {
    auto cookie = Cookie("id", "initial");
    REQUIRE(cookie.get_value() == "initial");
    cookie.set_value("updated");
    REQUIRE(cookie.get_value() == "updated");
    cookie.set_domain("example.com");
    REQUIRE(cookie.get_domain() == "example.com");
    REQUIRE(cookie.get_path() == "/");
    cookie.set_path("/api");
    REQUIRE(cookie.get_path() == "/api");
    auto expiration = time_from_string("2025-01-15 12:30:00");
    cookie.set_expiration(expiration);
    REQUIRE(cookie.get_expiration() == expiration);
    REQUIRE(!cookie.is_secure());
    cookie.set_secure(true);
    REQUIRE(cookie.is_secure());
    cookie.set_secure(false);
    REQUIRE(!cookie.is_secure());
    REQUIRE(!cookie.is_http_only());
    cookie.set_http_only(true);
    REQUIRE(cookie.is_http_only());
    cookie.set_http_only(false);
    REQUIRE(!cookie.is_http_only());
  }

  TEST_CASE("stream_basic") {
    auto cookie = Cookie("name", "value");
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() == "name=value; Path=/");
  }

  TEST_CASE("stream_with_domain") {
    auto cookie = Cookie("id", "abc");
    cookie.set_domain("test.com");
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() == "id=abc; Domain=test.com; Path=/");
  }

  TEST_CASE("stream_with_custom_path") {
    auto cookie = Cookie("token", "xyz");
    cookie.set_path("/secure");
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() == "token=xyz; Path=/secure");
  }

  TEST_CASE("stream_with_expiration") {
    auto cookie = Cookie("session", "data");
    auto expiration = time_from_string("2025-02-20 14:05:30");
    cookie.set_expiration(expiration);
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() ==
      "session=data; Path=/; Expires=Thu, 20 Feb 2025 14:05:30 GMT");
  }

  TEST_CASE("stream_with_secure") {
    auto cookie = Cookie("secure_token", "secret");
    cookie.set_secure(true);
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() == "secure_token=secret; Path=/; Secure");
  }

  TEST_CASE("stream_with_http_only") {
    auto cookie = Cookie("http_token", "data");
    cookie.set_http_only(true);
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() == "http_token=data; Path=/; HttpOnly");
  }

  TEST_CASE("stream_with_all_attributes") {
    auto cookie = Cookie("full", "complete");
    cookie.set_domain("secure.example.com");
    cookie.set_path("/api/v1");
    auto expiration = time_from_string("2025-12-31 23:59:59");
    cookie.set_expiration(expiration);
    cookie.set_secure(true);
    cookie.set_http_only(true);
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() ==
      "full=complete; Domain=secure.example.com; Path=/api/v1; "
      "Expires=Wed, 31 Dec 2025 23:59:59 GMT; Secure; HttpOnly");
  }

  TEST_CASE("expiration_formatting_single_digit_hours") {
    auto cookie = Cookie("early", "morning");
    auto expiration = time_from_string("2025-03-10 03:07:02");
    cookie.set_expiration(expiration);
    auto ss = std::stringstream();
    ss << cookie;
    REQUIRE(ss.str() ==
      "early=morning; Path=/; Expires=Mon, 10 Mar 2025 03:07:02 GMT");
  }
}
