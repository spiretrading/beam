#include <doctest/doctest.h>
#include "Beam/WebServices/HttpServerPredicates.hpp"

using namespace Beam;

TEST_SUITE("HttpServerPredicates") {
  TEST_CASE("match_any") {
    auto predicate = match_any();
    auto get_request = HttpRequest("http://example.com/path");
    auto post_request =
      HttpRequest(HttpMethod::POST, "http://example.com/other");
    auto put_request = HttpRequest(HttpMethod::PUT, "http://example.com");
    REQUIRE(predicate(get_request));
    REQUIRE(predicate(post_request));
    REQUIRE(predicate(put_request));
  }

  TEST_CASE("match_any_with_method") {
    auto predicate = match_any(HttpMethod::POST);
    auto post_request = HttpRequest(HttpMethod::POST, "http://example.com");
    auto get_request = HttpRequest("http://example.com");
    REQUIRE(predicate(post_request));
    REQUIRE(!predicate(get_request));
  }

  TEST_CASE("match_any_get") {
    auto predicate = match_any(HttpMethod::GET);
    auto get_request = HttpRequest("http://example.com/path");
    auto post_request = HttpRequest(HttpMethod::POST, "http://example.com");
    REQUIRE(predicate(get_request));
    REQUIRE(!predicate(post_request));
  }

  TEST_CASE("matches_path") {
    auto predicate = matches_path(HttpMethod::GET, "/api/resource");
    auto matching_request = HttpRequest("http://example.com/api/resource");
    auto wrong_path = HttpRequest("http://example.com/other");
    auto wrong_method =
      HttpRequest(HttpMethod::POST, "http://example.com/api/resource");
    REQUIRE(predicate(matching_request));
    REQUIRE(!predicate(wrong_path));
    REQUIRE(!predicate(wrong_method));
  }

  TEST_CASE("matches_path_root") {
    auto predicate = matches_path(HttpMethod::GET, "/");
    auto root_request = HttpRequest("http://example.com/");
    auto other_request = HttpRequest("http://example.com/path");
    REQUIRE(predicate(root_request));
    REQUIRE(!predicate(other_request));
  }

  TEST_CASE("matches_path_with_query") {
    auto predicate = matches_path(HttpMethod::GET, "/search");
    auto request_with_query = HttpRequest("http://example.com/search?q=test");
    REQUIRE(predicate(request_with_query));
  }
}
