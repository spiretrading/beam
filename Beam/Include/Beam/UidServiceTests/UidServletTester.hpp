#ifndef BEAM_UIDSERVLETTESTER_HPP
#define BEAM_UIDSERVLETTESTER_HPP
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace UidService {
namespace Tests {

  /*! \class UidServletTester
      \brief Tests the UidServlet class.
   */
  class UidServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceProtocolServer.
      using UidServletContainer =
        Services::Tests::TestServiceProtocolServletContainer<
        MetaUidServlet<std::shared_ptr<LocalUidDataStore>>>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests reserving a block of uids.
      void TestReserveUidsService();

    private:
      std::shared_ptr<LocalUidDataStore> m_dataStore;
      boost::optional<UidServletContainer> m_container;
      boost::optional<Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;

      CPPUNIT_TEST_SUITE(UidServletTester);
        CPPUNIT_TEST(TestReserveUidsService);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
