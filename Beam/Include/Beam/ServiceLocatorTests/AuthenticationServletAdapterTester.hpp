#ifndef BEAM_AUTHENTICATIONSERVLETADAPTERTESTER_HPP
#define BEAM_AUTHENTICATIONSERVLETADAPTERTESTER_HPP
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServlet.hpp"
#include "Beam/ServicesTests/TestServlet.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class AuthenticationServletAdapterTester
      \brief Tests the AuthenticationServletAdapter class.
   */
  class AuthenticationServletAdapterTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using TestServiceLocatorClient =
        ServiceLocatorClient<Services::Tests::TestServiceProtocolClientBuilder>;

      //! The result of adapting the TestServlet with the
      //! AuthenticationServletAdapter.
      using MetaServlet = MetaAuthenticationServletAdapter<
        Services::Tests::MetaTestServlet,
        std::unique_ptr<TestServiceLocatorClient>>;

      //! The type of ServiceProtocolServletContainer.
      using ServiceProtocolServletContainer =
        Services::Tests::TestServiceProtocolServletContainer<MetaServlet>;

      //! The type of Servlet.
      using Servlet = AuthenticationServletAdapter<
        ServiceProtocolServletContainer,
        Services::Tests::TestServlet<ServiceProtocolServletContainer>,
        std::unique_ptr<TestServiceLocatorClient>>;

      //! The type of ServiceProtocolServer.
      using ServiceLocatorContainer =
        Services::Tests::TestServiceProtocolServletContainer<
        MetaServiceLocatorServlet<
        std::shared_ptr<LocalServiceLocatorDataStore>>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests requesting a service without authentication.
      void TestServiceWithoutAuthentication();

      //! Tests requesting a service with authentication.
      void TestServiceWithAuthentication();

    private:
      std::shared_ptr<LocalServiceLocatorDataStore> m_dataStore;
      boost::optional<ServiceProtocolServletContainer> m_container;
      boost::optional<Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;
      boost::optional<ServiceLocatorContainer> m_serviceLocatorContainer;
      boost::optional<TestServiceLocatorClient> m_serviceLocatorClient;

      CPPUNIT_TEST_SUITE(AuthenticationServletAdapterTester);
        CPPUNIT_TEST(TestServiceWithoutAuthentication);
        CPPUNIT_TEST(TestServiceWithAuthentication);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
