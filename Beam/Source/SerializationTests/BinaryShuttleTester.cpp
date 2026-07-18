module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"

module Beam;

#include "Beam/SerializationTests/ShuttleTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("BinaryShuttle") {
  TEST_CASE_TEMPLATE_INVOKE(ShuttleTestSuite, BinarySender<SharedBuffer>);
}
