#ifndef BEAM_MESSAGEPROTOCOLTESTER_HPP
#define BEAM_MESSAGEPROTOCOLTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ServicesTests/ServicesTests.hpp"

namespace Beam {
namespace Services {
namespace Tests {

  /*! \class MessageProtocolTester
      \brief Tests the MessageProtocol class.
   */
  class MessageProtocolTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests sending a message.
      void TestSendMessage();

      //! Tests receiving a message.
      void TestReceiveMessage();

    private:
      CPPUNIT_TEST_SUITE(MessageProtocolTester);
        CPPUNIT_TEST(TestSendMessage);
        CPPUNIT_TEST(TestReceiveMessage);
      CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
