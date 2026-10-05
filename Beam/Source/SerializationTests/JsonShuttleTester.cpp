#include <cstdint>
#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"
#include "Beam/SerializationTests/ShuttleTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("JsonShuttle") {
  TEST_CASE_TEMPLATE_INVOKE(ShuttleTestSuite, JsonSender<SharedBuffer>);

  TEST_CASE("from_json") {
    auto values = std::vector<ClassWithShuttleMethod>({
      ClassWithShuttleMethod('a', 123, 4.5),
      ClassWithShuttleMethod('b', -456, 0.125)});
    auto encoded = to_json(values);
    REQUIRE(from_json<decltype(values)>(encoded) == values);
    auto framed = "prefix" + encoded + "suffix";
    REQUIRE(from_json<decltype(values)>(
      std::string_view(framed).substr(6, encoded.size())) == values);
    REQUIRE(from_json<decltype(values)>(parse<JsonValue>(encoded)) == values);
    REQUIRE(from_json<std::string>("\"text\"") == "text");
    REQUIRE(from_json<std::string>(JsonValue("text")) == "text");
    REQUIRE_THROWS_AS(from_json<int>("{"), SerializationException);
    REQUIRE_THROWS_AS(from_json<int>(JsonValue("text")),
      SerializationException);
  }

  TEST_CASE("shuttle_integer_beyond_double_precision") {
    auto value = std::int64_t(9007199254740993);
    auto buffer = SharedBuffer();
    auto sender = JsonSender<SharedBuffer>();
    sender.set(Ref(buffer));
    sender.shuttle(value);
    REQUIRE(std::string(buffer.get_data(), buffer.get_size()) ==
      "\"9007199254740993\"");
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = std::int64_t();
    receiver.shuttle(received);
    REQUIRE(received == value);
  }
}
