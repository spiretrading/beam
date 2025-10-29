#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Services/HeartbeatMessage.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  using TestServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<NullChannel>, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;
}

TEST_SUITE("HeartbeatMessage") {
  TEST_CASE("emit") {
    auto client =
      TestServiceProtocolClient(std::make_unique<NullChannel>(), init());
    auto message = HeartbeatMessage<TestServiceProtocolClient>();
    REQUIRE_NOTHROW(message.emit(nullptr, Ref(client)));
  }

  TEST_CASE("stream") {
    auto client =
      TestServiceProtocolClient(std::make_unique<NullChannel>(), init());
    auto message = HeartbeatMessage<TestServiceProtocolClient>();
    test_round_trip_shuttle(message, [&] (auto&& received) {
    });
  }
}
