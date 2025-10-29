#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  using NullServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<NullChannel>, BinarySender<SharedBuffer>,
      NullEncoder>, TriggerTimer>;
}

TEST_SUITE("RequestToken") {
  TEST_CASE("construction") {
    auto client =
      NullServiceProtocolClient(std::make_unique<NullChannel>(), init());
    auto token =
      RequestToken<NullServiceProtocolClient, VoidService>(Ref(client), 1);
    REQUIRE(&token.get_client() == &client);
    REQUIRE(&token.get_session() == &client.get_session());
  }
}
