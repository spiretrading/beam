#include <limits>
#include <optional>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleJsonValue.hpp"
#include "Beam/Serialization/ShuttleOptional.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  class PrivateValue {
    public:
      int m_value;

      explicit PrivateValue(int value)
        : m_value(value) {}

    private:
      friend class Beam::DataShuttle;

      PrivateValue()
        : m_value(0) {}

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version) {
        shuttle.shuttle("value", m_value);
      }
  };

  struct OptionalValues {
    std::optional<JsonValue> m_missing;
    std::optional<JsonValue> m_null;
    std::optional<JsonValue> m_value;
    std::optional<int> m_count;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("missing", m_missing);
      shuttle.shuttle("null", m_null);
      shuttle.shuttle("value", m_value);
      shuttle.shuttle("count", m_count);
    }
  };
}

TEST_SUITE("JsonValueShuttle") {
  TEST_CASE("binary") {
    auto object = JsonObject();
    object.set("control\x01", "Value\x02\n\"quoted\"\\file");
    object.set("maximum", std::numeric_limits<double>::max());
    object.set("minimum", std::numeric_limits<double>::denorm_min());
    object.set("array", std::vector<JsonValue>({JsonNull(), false, true, 0,
      0.000000125, "text", JsonObject()}));
    test_round_trip_shuttle(JsonValue(object));
  }

  TEST_CASE("json_optional_values") {
    auto object = JsonObject();
    object.set("decimal", 0.000000125);
    object.set("flag", false);
    object.set("text", "Line\n\"quoted\"\\file");
    auto values = OptionalValues();
    values.m_null = JsonValue();
    values.m_value = object;
    values.m_count = 0;
    auto buffer = from<SharedBuffer>(to_json(values));
    auto encoded = parse<JsonValue>(buffer);
    auto& fields = std::get<JsonObject>(encoded);
    REQUIRE(!fields.get("missing"));
    REQUIRE(std::get_if<JsonNull>(&fields.at("null")));
    REQUIRE(fields.at("value") == JsonValue(object));
    REQUIRE(fields.at("count") == 0);
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = OptionalValues();
    received.m_missing = "previous";
    receiver.shuttle(received);
    REQUIRE(!received.m_missing);
    REQUIRE(received.m_null == values.m_null);
    REQUIRE(received.m_value == values.m_value);
    REQUIRE(received.m_count == values.m_count);
    test_round_trip_shuttle(values, [&] (const auto& received) {
      REQUIRE(parse<JsonValue>(to_json(received)) ==
        parse<JsonValue>(to_json(values)));
    });
    auto array = std::vector<std::optional<int>>({std::nullopt, 0, 5});
    buffer = from<SharedBuffer>(to_json(array));
    REQUIRE(to_string(buffer) == "[null,0,5]");
    receiver.set(Ref(buffer));
    auto decoded = std::vector<std::optional<int>>();
    receiver.shuttle(decoded);
    REQUIRE(decoded == array);
  }

  TEST_CASE("native_json") {
    auto object = JsonObject();
    object.set("control\x01", "Value\x02\n\"quoted\"\\file");
    object.set("maximum", std::numeric_limits<double>::max());
    object.set("minimum", std::numeric_limits<double>::denorm_min());
    object.set("array", std::vector<JsonValue>({JsonNull(), false, true, 0,
      0.000000125, "text", JsonObject()}));
    auto value = JsonValue(object);
    auto buffer = from<SharedBuffer>(to_json(value));
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = JsonValue();
    receiver.shuttle(received);
    REQUIRE(received == value);
  }

  TEST_CASE("unicode_escapes") {
    auto encoded = std::string(
      "\"\\u0000\\u0001\\u001f\\u007f\\u00e9\\u20ac\\ud83d\\ude00\"");
    auto expected = std::string("\0\x01\x1f\x7f", 4) +
      "\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80";
    REQUIRE(std::get<std::string>(parse<JsonValue>(encoded)) == expected);
    auto buffer = from<SharedBuffer>(to_json(JsonValue(expected)));
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = JsonValue();
    receiver.shuttle(received);
    REQUIRE(std::get<std::string>(received) == expected);
    auto source = to_parser_stream(encoded);
    REQUIRE(string_p.read(source));
    for(auto& encoded : {"\"\\u12\"", "\"\\uZZZZ\"", "\"\\uD800\"",
        "\"\\uDC00\"", "\"\\uD800\\u0041\""}) {
      auto source = to_parser_stream(encoded);
      auto value = std::string();
      REQUIRE(!string_p.read(source, value));
      source = to_parser_stream(encoded);
      REQUIRE(!string_p.read(source));
    }
  }

  TEST_CASE("optional_private_constructor") {
    auto value = std::optional<PrivateValue>(PrivateValue(37));
    auto buffer = from<SharedBuffer>(to_json(value));
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = std::optional<PrivateValue>();
    receiver.shuttle(received);
    REQUIRE(static_cast<bool>(received));
    REQUIRE(received->m_value == 37);
    test_round_trip_shuttle(value, [&] (const auto& received) {
      REQUIRE(static_cast<bool>(received));
      REQUIRE(received->m_value == 37);
    });
  }
}
