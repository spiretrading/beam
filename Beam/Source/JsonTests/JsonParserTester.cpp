#include <boost/lexical_cast.hpp>
#include <doctest/doctest.h>
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Parsers/Parse.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;

TEST_SUITE("JsonParser") {
  TEST_CASE("empty") {
    auto value = Parse<JsonValue>("{}");
    auto object = get<JsonObject>(&value);
    REQUIRE(object != nullptr);
    REQUIRE(lexical_cast<std::string>(*object) == "{}");
  }

  TEST_CASE("single_field") {
    auto value = Parse<JsonValue>("{\"a\":5}");
    auto object = get<JsonObject>(&value);
    REQUIRE(object != nullptr);
    REQUIRE((*object)["a"] == 5);
    REQUIRE(lexical_cast<std::string>(*object) == "{\"a\":5}");
  }
}
