module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/HeartbeatMessage.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

module Beam;

using namespace Beam;
using namespace Beam::Tests;

namespace {
  using TestProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<NullChannel>, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;
}

TEST_SUITE("HeartbeatMessage") {
  TEST_CASE("emit") {
    auto client =
      TestProtocolClient(std::make_unique<NullChannel>(), init());
    auto message = HeartbeatMessage<TestProtocolClient>();
    REQUIRE_NOTHROW(message.emit(nullptr, Ref(client)));
  }

  TEST_CASE("stream") {
    auto client =
      TestProtocolClient(std::make_unique<NullChannel>(), init());
    auto message = HeartbeatMessage<TestProtocolClient>();
    test_round_trip_shuttle(message, [&] (auto&& received) {
    });
  }
}
