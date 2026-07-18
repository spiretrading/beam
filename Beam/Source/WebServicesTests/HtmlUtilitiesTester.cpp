module;
#include "Prelude.hpp"
#include <doctest/doctest.h>

module Beam;

using namespace Beam;

TEST_SUITE("HtmlUtilities") {
  TEST_CASE("escape_empty") {
    REQUIRE(escape_html("") == "");
  }

  TEST_CASE("escape_no_special_characters") {
    REQUIRE(escape_html("hello world") == "hello world");
  }

  TEST_CASE("escape_ampersand") {
    REQUIRE(escape_html("a&b") == "a&amp;b");
  }

  TEST_CASE("escape_less_than") {
    REQUIRE(escape_html("a<b") == "a&lt;b");
  }

  TEST_CASE("escape_greater_than") {
    REQUIRE(escape_html("a>b") == "a&gt;b");
  }

  TEST_CASE("escape_double_quote") {
    REQUIRE(escape_html("a\"b") == "a&quot;b");
  }

  TEST_CASE("escape_single_quote") {
    REQUIRE(escape_html("a'b") == "a&#39;b");
  }

  TEST_CASE("escape_all_special_characters") {
    REQUIRE(escape_html("<div class=\"a&b\">it's</div>") ==
      "&lt;div class=&quot;a&amp;b&quot;&gt;it&#39;s&lt;/div&gt;");
  }

  TEST_CASE("escape_multiple_ampersands") {
    REQUIRE(escape_html("&&&") == "&amp;&amp;&amp;");
  }

  TEST_CASE("unescape_empty") {
    REQUIRE(unescape_html("") == "");
  }

  TEST_CASE("unescape_no_entities") {
    REQUIRE(unescape_html("hello world") == "hello world");
  }

  TEST_CASE("unescape_ampersand") {
    REQUIRE(unescape_html("a&amp;b") == "a&b");
  }

  TEST_CASE("unescape_less_than") {
    REQUIRE(unescape_html("a&lt;b") == "a<b");
  }

  TEST_CASE("unescape_greater_than") {
    REQUIRE(unescape_html("a&gt;b") == "a>b");
  }

  TEST_CASE("unescape_double_quote") {
    REQUIRE(unescape_html("a&quot;b") == "a\"b");
  }

  TEST_CASE("unescape_single_quote") {
    REQUIRE(unescape_html("a&#39;b") == "a'b");
  }

  TEST_CASE("unescape_all_entities") {
    REQUIRE(unescape_html(
      "&lt;div class=&quot;a&amp;b&quot;&gt;it&#39;s&lt;/div&gt;") ==
      "<div class=\"a&b\">it's</div>");
  }

  TEST_CASE("unescape_unknown_entity") {
    REQUIRE(unescape_html("&unknown;") == "&unknown;");
  }

  TEST_CASE("unescape_lone_ampersand") {
    REQUIRE(unescape_html("a & b") == "a & b");
  }

  TEST_CASE("roundtrip") {
    auto input = std::string("<script>alert('xss' & \"more\")</script>");
    REQUIRE(unescape_html(escape_html(input)) == input);
  }
}
