#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/SerializationTests/ShuttleTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("JsonShuttle") {
  TEST_CASE_TEMPLATE_INVOKE(ShuttleTestSuite, JsonSender<SharedBuffer>);
}
