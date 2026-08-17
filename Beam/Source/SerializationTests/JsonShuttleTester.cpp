#include <cstdint>
#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/SerializationTests/ShuttleTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("JsonShuttle") {
  TEST_CASE_TEMPLATE_INVOKE(ShuttleTestSuite, JsonSender<SharedBuffer>);

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
