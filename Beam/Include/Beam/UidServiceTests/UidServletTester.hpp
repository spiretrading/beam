#ifndef BEAM_UIDSERVLETTESTER_HPP
#define BEAM_UIDSERVLETTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Pointers/UniquePointerPolicy.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace UidService {
namespace Tests {

  /*! \class UidServletTester
      \brief Tests the UidServlet class.
   */
  class UidServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using UidServletContainer = Services::ServiceProtocolServletContainer<
        MetaUidServlet<std::shared_ptr<LocalUidDataStore>>,
        std::shared_ptr<ServerConnection>,
        Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;

      //! The type of Channel from the client to the server.
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;

      //! The type of ServiceProtocol on the client side.
      using ClientServiceProtocolClient = Services::ServiceProtocolClient<
        Services::MessageProtocol<ClientChannel,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests reserving a block of uids.
      void TestReserveUidsService();

    private:
      std::shared_ptr<LocalUidDataStore> m_dataStore;
      std::shared_ptr<ServerConnection> m_serverConnection;
      DelayPtr<UidServletContainer> m_container;
      DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(UidServletTester);
        CPPUNIT_TEST(TestReserveUidsService);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
