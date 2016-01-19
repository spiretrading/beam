#ifndef BEAM_REGISTRYSERVLETTESTER_HPP
#define BEAM_REGISTRYSERVLETTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/RegistryService/LocalRegistryDataStore.hpp"
#include "Beam/RegistryService/RegistryServlet.hpp"
#include "Beam/RegistryServiceTests/RegistryServiceTests.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace RegistryService {
namespace Tests {

  /*! \class RegistryServletTester
      \brief Tests the RegistryServlet class.
   */
  class RegistryServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using ServiceLocatorClient =
        ServiceLocator::Tests::ServiceLocatorTestInstance::ServiceLocatorClient;

      //! The type of ServerConnection.
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServletContainer = Services::ServiceProtocolServletContainer<
        ServiceLocator::MetaAuthenticationServletAdapter<
        MetaRegistryServlet<std::shared_ptr<LocalRegistryDataStore>>,
        std::unique_ptr<ServiceLocatorClient>>, ServerConnection*,
        Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
        Codecs::NullEncoder, std::shared_ptr<Threading::TriggerTimer>>;

      //! The type of Channel from the client to the server.
      using ClientChannel = IO::LocalClientChannel<IO::SharedBuffer>;

      //! The type of ServiceProtocol on the client side.
      using ClientServiceProtocolClient = Services::ServiceProtocolClient<
        Services::MessageProtocol<ClientChannel,
        Serialization::BinarySender<IO::SharedBuffer>,
        Codecs::NullEncoder>, Threading::TriggerTimer>;

      virtual void setUp();

      virtual void tearDown();

      //! Test making a directory RegistryEntry.
      void TestMakeDirectory();

      //! Test making a registry value.
      void TestMakeValue();

      //! Test loading a path.
      void TestLoadPath();

      //! Test loading a registry value.
      void TestLoadValue();

    private:
      DelayPtr<ServiceLocator::Tests::ServiceLocatorTestInstance>
        m_serviceLocatorInstance;
      std::shared_ptr<LocalRegistryDataStore> m_dataStore;
      DelayPtr<ServerConnection> m_serverConnection;
      DelayPtr<ServletContainer> m_container;
      DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      CPPUNIT_TEST_SUITE(RegistryServletTester);
        CPPUNIT_TEST(TestMakeDirectory);
        CPPUNIT_TEST(TestMakeValue);
        CPPUNIT_TEST(TestLoadPath);
        CPPUNIT_TEST(TestLoadValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
