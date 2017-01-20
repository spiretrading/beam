#ifndef BEAM_SERVICELOCATORSERVLETTESTER_HPP
#define BEAM_SERVICELOCATORSERVLETTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTests.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class ServiceLocatorServletTester
      \brief Tests the ServiceLocatorServlet class.
   */
  class ServiceLocatorServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServerConnection.
      using ServerConnection = IO::LocalServerConnection<IO::SharedBuffer>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServletContainer =
        Services::ServiceProtocolServletContainer<MetaServiceLocatorServlet<
        std::shared_ptr<LocalServiceLocatorDataStore>>,
        std::shared_ptr<ServerConnection>,
        Serialization::BinarySender<IO::SharedBuffer>,
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

      //! Test logging in as an account that doesn't exist.
      void TestLoginWithInvalidAccount();

      //! Test logging in with the wrong password.
      void TestLoginWithInvalidPassword();

      //! Tests a valid login.
      void TestValidLogin();

      //! Tests authenticating an account without logging in.
      void TestAuthenticateAccountWithoutLogin();

      //! Tests authenticating an invalid username.
      void TestAuthenticateAccountWithInvalidUsername();

      //! Tests authenticating an invalid password.
      void TestAuthenticateAccountWithInvalidPassword();

      //! Tests authenticating an account with invalid permissions.
      void TestAuthenticateAccountWithoutPermission();

      //! Tests a valid account authentication.
      void TestValidAuthenticateAccount();

      //! Tests session authentication without logging in.
      void TestSessionAuthenticationWithoutLogin();

      //! Tests session authentication with an invalid session id.
      void TestSessionAuthenticationWithInvalidSessionId();

      //! Tests a valid session authentication.
      void TestValidSessionAuthentication();

      //! Tests registering a service without logging in.
      void TestRegisterServiceWithoutLogin();

      //! Tests registering a service.
      void TestRegisterService();

      //! Tests unregistering a service without logging in.
      void TestUnregisterServiceWithoutLogin();

      //! Tests unregistering an invalid service.
      void TestUnregisterInvalidService();

      //! Tests register/unregister together.
      void TestRegisterUnregisterService();

      //! Tests locating a service without logging in.
      void TestLocatingWithoutLogin();

      //! Tests locating a non-existing service.
      void TestLocatingNonExistingService();

      //! Tests locating a service with a single provider.
      void TestLocatingSingleProviderService();

      //! Tests locating a service with a multiple providers.
      void TestLocatingMultipleProviderService();

      //! Tests creating an account without logging in.
      void TestCreateAccountWithoutLogin();

      //! Tests creating an account without permissions.
      void TestCreateAccountWithoutPermissions();

      //! Tests creating an account using an unavailable name.
      void TestCreateAccountUnavailableName();

      //! Tests creating an account with an empty name.
      void TestCreateAccountEmptyName();

      //! Tests creating a valid account.
      void TestValidCreateAccount();

      //! Tests creating a directory without logging in.
      void TestCreateDirectoryWithoutLogin();

      //! Tests creating without permissions.
      void TestCreateDirectoryWithoutPermissions();

      //! Tests creating a directory with an empty name.
      void TestCreateDirectoryEmptyName();

      //! Tests creating a valid directory.
      void TestValidCreateDirectory();

      //! Tests deleting an entry without logging in.
      void TestDeleteEntryWithoutLogin();

      //! Tests deleting a non-existing entry.
      void TestDeleteNonExistingEntry();

      //! Tests deleting an account without permissions.
      void TestDeleteAccountWithoutPermissions();

      //! Tests deleting an account.
      void TestDeleteAccount();

      //! Tests deleting a directory without permissions.
      void TestDeleteDirectoryWithoutPermissions();

      //! Tests deleting a directory.
      void TestDeleteDirectory();

      //! Tests loading a DirectoryEntry from a path.
      void TestLoadDirectoryEntryFromPath();

    private:
      std::shared_ptr<LocalServiceLocatorDataStore> m_dataStore;
      std::shared_ptr<ServerConnection> m_serverConnection;
      DelayPtr<ServiceProtocolServletContainer> m_container;
      DelayPtr<ClientServiceProtocolClient> m_clientProtocol;

      DirectoryEntry CreateUser(const std::string& username,
        const std::string& password);
      void CreateAccountAndLogin(Out<DirectoryEntry> account,
        Out<std::string> sessionId);
      void CreateAdditionalClient(const std::string& username,
        const std::string& password, Out<DirectoryEntry> account,
        Out<std::string> sessionId,
        Out<DelayPtr<ClientServiceProtocolClient>> service);

      CPPUNIT_TEST_SUITE(ServiceLocatorServletTester);
        CPPUNIT_TEST(TestLoginWithInvalidAccount);
        CPPUNIT_TEST(TestLoginWithInvalidPassword);
        CPPUNIT_TEST(TestValidLogin);
        CPPUNIT_TEST(TestAuthenticateAccountWithoutLogin);
        CPPUNIT_TEST(TestAuthenticateAccountWithInvalidUsername);
        CPPUNIT_TEST(TestAuthenticateAccountWithInvalidPassword);
        CPPUNIT_TEST(TestAuthenticateAccountWithoutPermission);
        CPPUNIT_TEST(TestValidAuthenticateAccount);
        CPPUNIT_TEST(TestSessionAuthenticationWithoutLogin);
        CPPUNIT_TEST(TestSessionAuthenticationWithInvalidSessionId);
        CPPUNIT_TEST(TestValidSessionAuthentication);
        CPPUNIT_TEST(TestRegisterServiceWithoutLogin);
        CPPUNIT_TEST(TestRegisterService);
        CPPUNIT_TEST(TestUnregisterServiceWithoutLogin);
        CPPUNIT_TEST(TestUnregisterInvalidService);
        CPPUNIT_TEST(TestRegisterUnregisterService);
        CPPUNIT_TEST(TestLocatingWithoutLogin);
        CPPUNIT_TEST(TestLocatingNonExistingService);
        CPPUNIT_TEST(TestLocatingSingleProviderService);
        CPPUNIT_TEST(TestLocatingMultipleProviderService);
        CPPUNIT_TEST(TestCreateAccountWithoutLogin);
        CPPUNIT_TEST(TestCreateAccountWithoutPermissions);
        CPPUNIT_TEST(TestCreateAccountUnavailableName);
        CPPUNIT_TEST(TestCreateAccountEmptyName);
        CPPUNIT_TEST(TestValidCreateAccount);
        CPPUNIT_TEST(TestCreateDirectoryWithoutLogin);
        CPPUNIT_TEST(TestCreateDirectoryWithoutPermissions);
        CPPUNIT_TEST(TestCreateDirectoryEmptyName);
        CPPUNIT_TEST(TestValidCreateDirectory);
        CPPUNIT_TEST(TestDeleteEntryWithoutLogin);
        CPPUNIT_TEST(TestDeleteNonExistingEntry);
        CPPUNIT_TEST(TestDeleteAccountWithoutPermissions);
        CPPUNIT_TEST(TestDeleteAccount);
        CPPUNIT_TEST(TestDeleteDirectoryWithoutPermissions);
        CPPUNIT_TEST(TestDeleteDirectory);
        CPPUNIT_TEST(TestLoadDirectoryEntryFromPath);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
