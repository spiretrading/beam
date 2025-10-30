#include <doctest/doctest.h>
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("JsonParser") {
  TEST_CASE("empty") {
    auto value = parse<JsonValue>("{}");
    auto object = get<JsonObject>(&value);
    REQUIRE(object);
    REQUIRE(to_string(*object) == "{}");
  }

  TEST_CASE("single_field") {
    auto value = parse<JsonValue>("{\"a\":5}");
    auto object = get<JsonObject>(&value);
    REQUIRE(object);
    REQUIRE((*object)["a"] == 5);
    REQUIRE(to_string(*object) == "{\"a\":5}");
  }
}
