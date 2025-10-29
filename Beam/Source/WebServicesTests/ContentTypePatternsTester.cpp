#include <doctest/doctest.h>
#include "Beam/WebServices/ContentTypePatterns.hpp"

using namespace Beam;

TEST_SUITE("ContentTypePatterns") {
  TEST_CASE("construct") {
    auto patterns = ContentTypePatterns();
    auto path = std::filesystem::path("unknown.xyz");
    auto content_type = patterns.get_content_type(path);
    REQUIRE(content_type.empty());
  }

  TEST_CASE("get_default_patterns") {
    auto patterns = ContentTypePatterns::get_default_patterns();
    auto css = patterns.get_content_type(std::filesystem::path("style.css"));
    REQUIRE(css == "text/css");
    auto html = patterns.get_content_type(std::filesystem::path("page.html"));
    REQUIRE(html == "text/html");
    auto js = patterns.get_content_type(std::filesystem::path("script.js"));
    REQUIRE(js == "application/javascript");
    auto svg = patterns.get_content_type(std::filesystem::path("icon.svg"));
    REQUIRE(svg == "image/svg+xml");
  }

  TEST_CASE("add_extension") {
    auto patterns = ContentTypePatterns();
    patterns.add_extension("json", "application/json");
    auto json = patterns.get_content_type(std::filesystem::path("data.json"));
    REQUIRE(json == "application/json");
    patterns.add_extension("xml", "application/xml");
    auto xml = patterns.get_content_type(std::filesystem::path("config.xml"));
    REQUIRE(xml == "application/xml");
  }

  TEST_CASE("get_content_type_with_path") {
    auto patterns = ContentTypePatterns::get_default_patterns();
    auto nested = patterns.get_content_type(
      std::filesystem::path("assets/styles/main.css"));
    REQUIRE(nested == "text/css");
    auto root = patterns.get_content_type(std::filesystem::path("/index.html"));
    REQUIRE(root == "text/html");
  }

  TEST_CASE("unknown_extension") {
    auto patterns = ContentTypePatterns::get_default_patterns();
    auto unknown =
      patterns.get_content_type(std::filesystem::path("file.unknown"));
    REQUIRE(unknown.empty());
    auto no_extension =
      patterns.get_content_type(std::filesystem::path("README"));
    REQUIRE(no_extension.empty());
  }

  TEST_CASE("case_sensitive_extension") {
    auto patterns = ContentTypePatterns::get_default_patterns();
    auto uppercase =
      patterns.get_content_type(std::filesystem::path("style.CSS"));
    REQUIRE(uppercase.empty());
    patterns.add_extension("CSS", "text/css");
    auto added = patterns.get_content_type(std::filesystem::path("style.CSS"));
    REQUIRE(added == "text/css");
  }

  TEST_CASE("override_extension") {
    auto patterns = ContentTypePatterns();
    patterns.add_extension("txt", "text/plain");
    auto first = patterns.get_content_type(std::filesystem::path("file.txt"));
    REQUIRE(first == "text/plain");
    patterns.add_extension("txt", "text/plain; charset=utf-8");
    auto overridden =
      patterns.get_content_type(std::filesystem::path("file.txt"));
    REQUIRE(overridden == "text/plain; charset=utf-8");
  }
}
