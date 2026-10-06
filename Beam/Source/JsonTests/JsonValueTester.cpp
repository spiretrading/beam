#include <cmath>
#include <limits>
#include <locale>
#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Json/JsonParser.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace boost;

namespace {
  struct DecimalComma : std::numpunct<char> {
    char do_decimal_point() const override {
      return ',';
    }
  };
}

TEST_SUITE("JsonValue") {
  TEST_CASE("object_copy") {
    auto original = get<JsonObject>(parse<JsonValue>(
      R"({"name":"original","nested":{"count":1},"array":[{"count":2}]})"));
    auto copy = JsonObject();
    SUBCASE("constructor") {
      auto constructed = JsonObject(original);
      copy = std::move(constructed);
    }
    SUBCASE("assignment") {
      copy.set("obsolete", true);
      copy = original;
      REQUIRE(!copy.get("obsolete"));
    }
    REQUIRE(copy == original);
    copy.set("name", "copy");
    get<JsonObject>(copy["nested"])["count"] = 10;
    get<JsonObject>(get<std::vector<JsonValue>>(copy["array"])[0]).set(
      "count", 20);
    REQUIRE(original.at("name") == JsonValue("original"));
    REQUIRE(
      get<JsonObject>(original.at("nested")).at("count") == JsonValue(1));
    REQUIRE(get<JsonObject>(get<std::vector<JsonValue>>(
      original.at("array"))[0]).at("count") == JsonValue(2));
    original["name"] = "changed";
    REQUIRE(copy.at("name") == JsonValue("copy"));
    copy = copy;
    REQUIRE(copy.at("name") == JsonValue("copy"));
  }

  TEST_CASE("value_copy") {
    auto original = parse<JsonValue>(R"({"array":[{"count":1}]})");
    auto copy = JsonValue();
    SUBCASE("constructor") {
      auto constructed = JsonValue(original);
      copy = std::move(constructed);
    }
    SUBCASE("assignment") {
      copy = "replaced";
      copy = original;
    }
    REQUIRE(copy == original);
    auto& array = get<std::vector<JsonValue>>(get<JsonObject>(copy)["array"]);
    get<JsonObject>(array[0])["count"] = 2;
    REQUIRE(get<JsonObject>(get<std::vector<JsonValue>>(
      get<JsonObject>(original).at("array"))[0]).at("count") == JsonValue(1));
    REQUIRE(get<JsonObject>(array[0]).at("count") == JsonValue(2));
  }

  TEST_CASE("strings") {
    auto value = JsonValue("Line one\n\"Line two\"\\file\t\b\f\r");
    REQUIRE(to_string(value) ==
      "\"Line one\\n\\\"Line two\\\"\\\\file\\t\\b\\f\\r\"");
    REQUIRE(parse<JsonValue>(to_string(value)) == value);
    auto object = JsonObject();
    object.set("quoted\"name\\\n", value);
    auto encoded = to_string(object);
    REQUIRE(encoded == "{\"quoted\\\"name\\\\\\n\":" +
      to_string(value) + "}");
    REQUIRE(parse<JsonValue>(encoded) == JsonValue(object));
    REQUIRE(to_string(JsonValue(std::string("\0\x01\x1f", 3))) ==
      "\"\\u0000\\u0001\\u001f\"");
  }

  TEST_CASE("numbers") {
    for(auto number : {0.0, -0.0, 0.000000125, 1.2345678901234567,
        1234567890123.0, std::numeric_limits<double>::max(),
        std::numeric_limits<double>::denorm_min()}) {
      auto encoded = to_string(JsonValue(number));
      auto decoded = double();
      auto result = std::from_chars(
        encoded.data(), encoded.data() + encoded.size(), decoded);
      REQUIRE(result.ec == std::errc());
      REQUIRE(decoded == number);
      REQUIRE(result.ptr == encoded.data() + encoded.size());
    }
    auto output = std::stringstream();
    output.imbue(std::locale(std::locale::classic(), new DecimalComma()));
    JsonValue(1.25).save(output);
    REQUIRE(output.str() == "1.25");
    REQUIRE_THROWS_AS(to_string(JsonValue(
      std::numeric_limits<double>::infinity())), std::invalid_argument);
    REQUIRE_THROWS_AS(to_string(JsonValue(
      std::numeric_limits<double>::quiet_NaN())), std::invalid_argument);
  }
}
