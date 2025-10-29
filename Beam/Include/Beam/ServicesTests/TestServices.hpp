#ifndef BEAM_TEST_SERVICES_HPP
#define BEAM_TEST_SERVICES_HPP
#include <memory>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

namespace Beam::Tests {

  /** The type of ServiceProtocolClient used in tests. */
  using TestServiceProtocolClient = ServiceProtocolClient<MessageProtocol<
    std::unique_ptr<LocalClientChannel>, BinarySender<SharedBuffer>,
    NullEncoder>, TriggerTimer>;

  /** The type of ServiceProtocolClientBuilder used in tests. */
  using TestServiceProtocolClientBuilder = ServiceProtocolClientBuilder<
    TestServiceProtocolClient::MessageProtocol, TriggerTimer>;

  /** The type of ServiceProtocolServer used in tests. */
  using TestServiceProtocolServer = ServiceProtocolServer<
    std::shared_ptr<LocalServerConnection>, BinarySender<SharedBuffer>,
    NullEncoder, std::unique_ptr<TriggerTimer>>;

  /** The type of ServiceProtocolServletContainer used in tests. */
  template<typename MetaServlet,
    typename ServletPointerPolicy = LocalPointerPolicy>
  using TestServiceProtocolServletContainer = ServiceProtocolServletContainer<
    MetaServlet, std::shared_ptr<LocalServerConnection>,
    BinarySender<SharedBuffer>, NullEncoder, std::unique_ptr<TriggerTimer>,
    ServletPointerPolicy>;

  BEAM_DEFINE_SERVICES(test_services,
    (ConstantService, "Beam.Services.Tests.ConstantService", void),
    (VoidService, "Beam.Services.Tests.VoidService", void, (int, n)),
    (IdentityService, "Beam.Services.Tests.IdentityService", int, (int, n)),
    (AdditionService, "Beam.Services.Tests.AdditionService", int,
      (int, a), (int, b)));
}

#endif
