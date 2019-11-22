#ifndef BEAM_SERVICEPROTOCOLSERVLETCONTAINERTESTER_HPP
#define BEAM_SERVICEPROTOCOLSERVLETCONTAINERTESTER_HPP
#include <optional>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/ServicesTests/TestServlet.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Services {
namespace Tests {

  /*! \class ServiceProtocolServletContainerTester
      \brief Tests the ServiceProtocolServer class.
   */
  class ServiceProtocolServletContainerTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      typedef IO::LocalServerConnection<IO::SharedBuffer> ServerConnection;

      //! The type of Channel from the client to the server.
      typedef IO::LocalClientChannel<IO::SharedBuffer> ClientChannel;

      //! The type of ServiceProtocolServer.
      typedef Services::ServiceProtocolServletContainer<MetaTestServlet,
        std::unique_ptr<ServerConnection>, Serialization::BinarySender<
        IO::SharedBuffer>, Codecs::NullEncoder,
        std::shared_ptr<Threading::TriggerTimer>>
        ServiceProtocolServletContainer;

      //! The type of ServiceProtocolClient on the client side.
      typedef ServiceProtocolClient<MessageProtocol<ClientChannel,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer> ClientServiceProtocolClient;

      virtual void setUp();

      virtual void tearDown();

      //! Tests sending/receiving a service of type void.
      void TestVoidReturnType();

    private:
      std::optional<ServiceProtocolServletContainer> m_container;
      std::optional<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(ServiceProtocolServletContainerTester);
        CPPUNIT_TEST(TestVoidReturnType);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
