#include <utility>
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
    auto cases = {
      std::pair("style.css", "text/css"),
      std::pair("report.csv", "text/csv"),
      std::pair("image.gif", "image/gif"),
      std::pair("page.html", "text/html"),
      std::pair("icon.ico", "image/vnd.microsoft.icon"),
      std::pair("image.jpeg", "image/jpeg"),
      std::pair("image.jpg", "image/jpeg"),
      std::pair("script.js", "application/javascript"),
      std::pair("data.json", "application/json"),
      std::pair("document.markdown", "text/markdown"),
      std::pair("document.md", "text/markdown"),
      std::pair("module.mjs", "application/javascript"),
      std::pair("report.pdf", "application/pdf"),
      std::pair("image.png", "image/png"),
      std::pair("icon.svg", "image/svg+xml"),
      std::pair("document.txt", "text/plain"),
      std::pair("module.wasm", "application/wasm"),
      std::pair("image.webp", "image/webp"),
      std::pair("font.woff", "font/woff"),
      std::pair("font.woff2", "font/woff2"),
      std::pair("archive.zip", "application/zip")};
    for(auto& [filename, expected] : cases) {
      REQUIRE(patterns.get_content_type(std::filesystem::path(filename)) ==
        expected);
    }
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
