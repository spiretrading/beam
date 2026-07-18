module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include "Beam/SerializationTests/ShuttleTestSuite.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"

module Beam;

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("JsonShuttle") {
  TEST_CASE_TEMPLATE_INVOKE(ShuttleTestSuite, JsonSender<SharedBuffer>);
}
