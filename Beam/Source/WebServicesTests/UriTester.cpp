#include <sstream>
#include <doctest/doctest.h>
#include "Beam/WebServices/Uri.hpp"

using namespace Beam;

TEST_SUITE("Uri") {
  TEST_CASE("construct_empty") {
    auto uri = Uri();
    REQUIRE(uri.get_scheme().empty());
    REQUIRE(uri.get_username().empty());
    REQUIRE(uri.get_password().empty());
    REQUIRE(uri.get_hostname().empty());
    REQUIRE(uri.get_port() == 0);
    REQUIRE(uri.get_path().empty());
    REQUIRE(uri.get_query().empty());
    REQUIRE(uri.get_fragment().empty());
  }

  TEST_CASE("construct_simple_http") {
    auto uri = Uri("http://example.com");
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_port() == 80);
    REQUIRE(uri.get_path().empty());
  }

  TEST_CASE("construct_https") {
    auto uri = Uri("https://secure.example.com");
    REQUIRE(uri.get_scheme() == "https");
    REQUIRE(uri.get_hostname() == "secure.example.com");
    REQUIRE(uri.get_port() == 443);
  }

  TEST_CASE("construct_with_port") {
    auto uri = Uri("http://example.com:8080");
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_port() == 8080);
  }

  TEST_CASE("construct_with_path") {
    auto uri = Uri("http://example.com/path/to/resource");
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_path() == "/path/to/resource");
  }

  TEST_CASE("construct_with_query") {
    auto uri = Uri("http://example.com/search?q=test&page=1");
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_path() == "/search");
    REQUIRE(uri.get_query() == "q=test&page=1");
  }

  TEST_CASE("construct_with_fragment") {
    auto uri = Uri("http://example.com/page#section");
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_path() == "/page");
    REQUIRE(uri.get_fragment() == "section");
  }

  TEST_CASE("construct_with_credentials") {
    auto uri = Uri("http://user:pass@example.com");
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_username() == "user");
    REQUIRE(uri.get_password() == "pass");
    REQUIRE(uri.get_hostname() == "example.com");
  }

  TEST_CASE("construct_complete_uri") {
    auto uri = Uri("https://user:pass@example.com:8443/path?query=value#fragment");
    REQUIRE(uri.get_scheme() == "https");
    REQUIRE(uri.get_username() == "user");
    REQUIRE(uri.get_password() == "pass");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_port() == 8443);
    REQUIRE(uri.get_path() == "/path");
    REQUIRE(uri.get_query() == "query=value");
    REQUIRE(uri.get_fragment() == "fragment");
  }

  TEST_CASE("construct_websocket") {
    auto ws = Uri("ws://example.com");
    REQUIRE(ws.get_scheme() == "ws");
    REQUIRE(ws.get_port() == 80);
    auto wss = Uri("wss://secure.example.com");
    REQUIRE(wss.get_scheme() == "wss");
    REQUIRE(wss.get_port() == 443);
  }

  TEST_CASE("set_port") {
    auto uri = Uri("http://example.com");
    REQUIRE(uri.get_port() == 80);
    uri.set_port(9000);
    REQUIRE(uri.get_port() == 9000);
  }

  TEST_CASE("construct_from_char_pointer") {
    auto source = std::string("http://example.com/path");
    auto uri = Uri(source.c_str(), source.c_str() + source.size());
    REQUIRE(uri.get_scheme() == "http");
    REQUIRE(uri.get_hostname() == "example.com");
    REQUIRE(uri.get_path() == "/path");
  }

  TEST_CASE("malformed_uri_throws") {
    REQUIRE_THROWS_AS(Uri("not a valid uri!@#$%"), MalformedUriException);
  }

  TEST_CASE("port_overflow_throws") {
    REQUIRE_THROWS_AS(Uri("http://example.com:99999"), MalformedUriException);
  }

  TEST_CASE("stream_simple") {
    auto uri = Uri("http://example.com");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "http://example.com");
  }

  TEST_CASE("stream_with_port") {
    auto uri = Uri("http://example.com:8080");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "http://example.com:8080");
  }

  TEST_CASE("stream_with_path") {
    auto uri = Uri("http://example.com/path");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "http://example.com/path");
  }

  TEST_CASE("stream_with_query") {
    auto uri = Uri("http://example.com/search?q=test");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "http://example.com/search?q=test");
  }

  TEST_CASE("stream_with_fragment") {
    auto uri = Uri("http://example.com/page#section");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "http://example.com/page#section");
  }

  TEST_CASE("stream_with_credentials") {
    auto uri = Uri("http://user:pass@example.com");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "http://user:pass@example.com");
  }

  TEST_CASE("stream_complete") {
    auto uri = Uri("https://user:pass@example.com:8443/path?query=value#fragment");
    auto ss = std::stringstream();
    ss << uri;
    REQUIRE(ss.str() == "https://user:pass@example.com:8443/path?query=value#fragment");
  }

  TEST_CASE("relative_path") {
    auto uri = Uri("/relative/path");
    REQUIRE(uri.get_scheme().empty());
    REQUIRE(uri.get_hostname().empty());
    REQUIRE(uri.get_path() == "/relative/path");
  }

  TEST_CASE("path_only") {
    auto uri = Uri("path/without/slash");
    REQUIRE(uri.get_scheme().empty());
    REQUIRE(uri.get_hostname().empty());
    REQUIRE(uri.get_path() == "path/without/slash");
  }
}
