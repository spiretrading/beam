#include <doctest/doctest.h>
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("JsonParser") {
  TEST_CASE("empty") {
    auto value = parse<JsonValue>("{}");
    auto object = std::get_if<JsonObject>(&value);
    REQUIRE(object);
    REQUIRE(to_string(*object) == "{}");
  }

  TEST_CASE("single_field") {
    auto value = parse<JsonValue>("{\"a\":5}");
    auto object = std::get_if<JsonObject>(&value);
    REQUIRE(object);
    REQUIRE((*object)["a"] == 5);
    REQUIRE(to_string(*object) == "{\"a\":5}");
  }

  TEST_CASE("exponents") {
    auto value = parse<JsonValue>("1e25");
    REQUIRE(std::get<double>(value) == 1e25);
    value = parse<JsonValue>(R"({"values":[1e25,-1E+25,1.25e-3]})");
    auto& values = std::get<std::vector<JsonValue>>(
      std::get<JsonObject>(value).at("values"));
    REQUIRE(values == std::vector<JsonValue>({1e25, -1e25, 0.00125}));
    for(auto& text : {"[1e]", "[1e+]", R"({"value":1e-})",
        "[1e9999]", "[1e-9999]"}) {
      REQUIRE_THROWS_AS(parse<JsonValue>(text), ParserException);
    }
  }
}
