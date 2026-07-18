module;
#include "Prelude.hpp"

export module Beam:TestServices;

import :AuthenticationServletAdapter;
import :LocalServerConnection;
import :TriggerTimer;

export namespace Beam::Tests {

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

  /**
   * Instantiates types of ServiceProtocolServletContainers used for testing
   * that require authentication.
   */
  template<typename MetaServlet,
    typename ServletPointerPolicy = LocalPointerPolicy>
  using TestAuthenticatedServiceProtocolServletContainer =
    TestServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
      MetaServlet, ServiceLocatorClient, ServletPointerPolicy>>;

  BEAM_DEFINE_SERVICES(test_services,
    (ConstantService, "Beam.Services.Tests.ConstantService", void),
    (VoidService, "Beam.Services.Tests.VoidService", void, (int, n)),
    (IdentityService, "Beam.Services.Tests.IdentityService", int, (int, n)),
    (AdditionService, "Beam.Services.Tests.AdditionService", int,
      (int, a), (int, b)));
}

#define REQUIRE_NO_THROW(expression) \
  [&] { \
    REQUIRE_NOTHROW(return (expression)); \
    throw 0; \
  }()

