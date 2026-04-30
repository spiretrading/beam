#include <doctest/doctest.h>
#include "Beam/Utilities/ToString.hpp"
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
    REQUIRE(to_string(uri) == "http://example.com");
  }

  TEST_CASE("stream_with_port") {
    auto uri = Uri("http://example.com:8080");
    REQUIRE(to_string(uri) == "http://example.com:8080");
  }

  TEST_CASE("stream_with_path") {
    auto uri = Uri("http://example.com/path");
    REQUIRE(to_string(uri) == "http://example.com/path");
  }

  TEST_CASE("stream_with_query") {
    auto uri = Uri("http://example.com/search?q=test");
    REQUIRE(to_string(uri) == "http://example.com/search?q=test");
  }

  TEST_CASE("stream_with_fragment") {
    auto uri = Uri("http://example.com/page#section");
    REQUIRE(to_string(uri) == "http://example.com/page#section");
  }

  TEST_CASE("stream_with_credentials") {
    auto uri = Uri("http://user:pass@example.com");
    REQUIRE(to_string(uri) == "http://user:pass@example.com");
  }

  TEST_CASE("stream_complete") {
    auto uri = Uri("https://user:pass@example.com:8443/path?query=value#fragment");
    REQUIRE(to_string(uri) == "https://user:pass@example.com:8443/path?query=value#fragment");
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

TEST_SUITE("uri_encode") {
  TEST_CASE("empty_string") {
    REQUIRE(uri_encode("") == "");
  }

  TEST_CASE("unreserved_characters") {
    REQUIRE(uri_encode("ABCxyz012-_.~") == "ABCxyz012-_.~");
  }

  TEST_CASE("space") {
    REQUIRE(uri_encode("hello world") == "hello%20world");
  }

  TEST_CASE("special_characters") {
    REQUIRE(uri_encode("!@#$%") == "%21%40%23%24%25");
  }

  TEST_CASE("slash_and_colon") {
    REQUIRE(uri_encode("/account/profile") == "%2Faccount%2Fprofile");
  }

  TEST_CASE("query_characters") {
    REQUIRE(uri_encode("key=value&other=123") == "key%3Dvalue%26other%3D123");
  }
}

TEST_SUITE("uri_decode") {
  TEST_CASE("empty_string") {
    REQUIRE(uri_decode("") == "");
  }

  TEST_CASE("no_encoding") {
    REQUIRE(uri_decode("hello") == "hello");
  }

  TEST_CASE("plus_to_space") {
    REQUIRE(uri_decode("hello+world") == "hello world");
  }

  TEST_CASE("percent_encoded_space") {
    REQUIRE(uri_decode("hello%20world") == "hello world");
  }

  TEST_CASE("percent_encoded_special_characters") {
    REQUIRE(uri_decode("%21%40%23%24%25") == "!@#$%");
  }

  TEST_CASE("mixed_encoding") {
    REQUIRE(uri_decode("a+b%20c") == "a b c");
  }

  TEST_CASE("lowercase_hex") {
    REQUIRE(uri_decode("%2f%3a") == "/:");
  }

  TEST_CASE("uppercase_hex") {
    REQUIRE(uri_decode("%2F%3A") == "/:");
  }

  TEST_CASE("incomplete_percent_at_end") {
    REQUIRE(uri_decode("hello%2") == "hello%2");
  }

  TEST_CASE("percent_followed_by_non_hex") {
    REQUIRE(uri_decode("hello%GG") == "hello%GG");
  }

  TEST_CASE("path_with_query") {
    REQUIRE(uri_decode("%2Faccount%2Fprofile%3Fid%3D123") ==
      "/account/profile?id=123");
  }

  TEST_CASE("unreserved_characters_unchanged") {
    REQUIRE(uri_decode("ABCxyz012-_.~") == "ABCxyz012-_.~");
  }

  TEST_CASE("roundtrip") {
    auto original = std::string("/path?key=hello world&x=a+b");
    REQUIRE(uri_decode(uri_encode(original)) == original);
  }
}

TEST_SUITE("parse_query") {
  TEST_CASE("empty_string") {
    auto parameters = parse_query("");
    REQUIRE(parameters.empty());
  }

  TEST_CASE("single_parameter") {
    auto parameters = parse_query("key=value");
    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters.find("key")->second == "value");
  }

  TEST_CASE("multiple_parameters") {
    auto parameters = parse_query("a=1&b=2&c=3");
    REQUIRE(parameters.size() == 3);
    REQUIRE(parameters.find("a")->second == "1");
    REQUIRE(parameters.find("b")->second == "2");
    REQUIRE(parameters.find("c")->second == "3");
  }

  TEST_CASE("empty_value") {
    auto parameters = parse_query("key=");
    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters.find("key")->second == "");
  }

  TEST_CASE("no_value") {
    auto parameters = parse_query("key");
    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters.find("key")->second == "");
  }

  TEST_CASE("encoded_key_and_value") {
    auto parameters = parse_query("hello%20world=foo%26bar");
    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters.find("hello world")->second == "foo&bar");
  }

  TEST_CASE("plus_in_value") {
    auto parameters = parse_query("q=hello+world");
    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters.find("q")->second == "hello world");
  }

  TEST_CASE("duplicate_keys") {
    auto parameters = parse_query("id=1&id=2&id=3");
    REQUIRE(parameters.size() == 3);
    REQUIRE(parameters.count("id") == 3);
  }

  TEST_CASE("trailing_ampersand") {
    auto parameters = parse_query("a=1&b=2&");
    REQUIRE(parameters.size() == 2);
    REQUIRE(parameters.find("a")->second == "1");
    REQUIRE(parameters.find("b")->second == "2");
  }

  TEST_CASE("leading_ampersand") {
    auto parameters = parse_query("&a=1");
    REQUIRE(parameters.size() == 1);
    REQUIRE(parameters.find("a")->second == "1");
  }

  TEST_CASE("from_uri") {
    auto uri = Uri("http://example.com/search?q=test&page=1");
    auto parameters = parse_query(uri);
    REQUIRE(parameters.size() == 2);
    REQUIRE(parameters.find("q")->second == "test");
    REQUIRE(parameters.find("page")->second == "1");
  }
}
