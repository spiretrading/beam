#ifndef BEAM_SHUTTLEVARIANTTESTER_HPP
#define BEAM_SHUTTLEVARIANTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/SerializationTests/SerializationTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Serialization {
namespace Tests {

  /*! \class ShuttleVariantTester
      \brief Tests shuttling a boost::variant.
   */
  class ShuttleVariantTester : public CPPUNIT_NS::TestFixture {
    public:
      virtual void setUp();

      virtual void tearDown();

      //! Tests shuttling a variant with a single type.
      void TestSingleType();

      //! Tests shuttling the first member of a variant with a two types.
      void TestFirstMemberWithTwoTypes();

      //! Tests shuttling the second member of a variant with a two types.
      void TestSecondMemberWithTwoTypes();

      //! Tests shuttling the first member of a variant with a three types.
      void TestFirstMemberWithThreeTypes();

      //! Tests shuttling the second member of a variant with a three types.
      void TestSecondMemberWithThreeTypes();

      //! Tests shuttling the third member of a variant with a three types.
      void TestThirdMemberWithThreeTypes();

    private:
      BinaryReceiver<IO::SharedBuffer> m_receiver;
      BinarySender<IO::SharedBuffer> m_sender;
      IO::SharedBuffer m_buffer;

      CPPUNIT_TEST_SUITE(ShuttleVariantTester);
        CPPUNIT_TEST(TestSingleType);
        CPPUNIT_TEST(TestFirstMemberWithTwoTypes);
        CPPUNIT_TEST(TestSecondMemberWithTwoTypes);
        CPPUNIT_TEST(TestFirstMemberWithThreeTypes);
        CPPUNIT_TEST(TestSecondMemberWithThreeTypes);
        CPPUNIT_TEST(TestThirdMemberWithThreeTypes);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
