#include <memory>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleUniquePtr.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleUniquePtr") {
  TEST_CASE("unique_ptr_int") {
    test_round_trip_shuttle(std::make_unique<int>(123), [&] (auto&& received) {
      REQUIRE(received);
      REQUIRE(*received == 123);
    });
  }

  TEST_CASE("polymorphic_unique_ptr") {
    auto out_value = std::make_unique<PolymorphicDerivedClassA>();
    auto registry = TypeRegistry<BinarySender<SharedBuffer>>();
    registry.template add<PolymorphicDerivedClassA>("PolymorphicDerivedClassA");
    registry.template add<PolymorphicDerivedClassB>("PolymorphicDerivedClassB");
    using Sender = BinarySender<SharedBuffer>;
    using Receiver = inverse_t<Sender>;
    auto sender = Sender(Ref(registry));
    auto buffer = typename Sender::Sink();
    sender.set(Ref(buffer));
    sender.send(out_value.get());
    auto receiver = Receiver(Ref(registry));
    receiver.set(Ref(buffer));
    auto in_value = std::unique_ptr<PolymorphicBaseClass>();
    receiver.shuttle(in_value);
    REQUIRE(in_value);
    REQUIRE(in_value->to_string() == out_value->to_string());
  }
}
