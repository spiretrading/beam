#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/ShuttleClone.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleClone") {
  TEST_CASE("polymorphic_clone") {
    auto derived = PolymorphicDerivedClassA();
    auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
    registry.template add<PolymorphicDerivedClassA>(
      "PolymorphicDerivedClassA");
    registry.template add<PolymorphicDerivedClassB>(
      "PolymorphicDerivedClassB");
    auto sender = BinarySender<SharedBuffer>(Ref(registry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
    auto clone = shuttle_clone(
      static_cast<PolymorphicBaseClass&>(derived), sender, receiver);
    REQUIRE(clone);
    REQUIRE(clone->to_string() == derived.to_string());
  }
}
