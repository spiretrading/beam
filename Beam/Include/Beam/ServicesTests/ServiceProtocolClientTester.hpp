#ifndef BEAM_SERVICEPROTOCOLCLIENTTESTER_HPP
#define BEAM_SERVICEPROTOCOLCLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Services {
namespace Tests {

  /*! \class ServiceProtocolClientTester
      \brief Tests the ServiceProtocolClient class.
   */
  class ServiceProtocolClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;

      //! The type of Channel from the client to the server.
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;

      //! The type of ServiceProtocolClient on the server side.
      using ServerServiceProtocolClient = ServiceProtocolClient<
        MessageProtocol<std::unique_ptr<ServerConnection::Channel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      //! The type of ServiceProtocolClient on the client side.
      using ClientServiceProtocolClient = ServiceProtocolClient<
        MessageProtocol<ClientChannel,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      //! Tests sending/receiving a service of type void.
      void TestVoidReturnType();

      //! Tests sending a request and receiving an exception.
      void TestException();

      //! Tests sending a request before closing the Connection.
      void TestRequestBeforeConnectionClosed();

      //! Tests sending a request after the Connection has been closed.
      void TestRequestAfterConnectionClosed();

    private:
      CPPUNIT_TEST_SUITE(ServiceProtocolClientTester);
        CPPUNIT_TEST(TestVoidReturnType);
        CPPUNIT_TEST(TestException);
        CPPUNIT_TEST(TestRequestBeforeConnectionClosed);
        CPPUNIT_TEST(TestRequestAfterConnectionClosed);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
