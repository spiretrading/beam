#ifndef BEAM_SIZEDECLARATIVEENCODERTESTER_HPP
#define BEAM_SIZEDECLARATIVEENCODERTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class SizeDeclarativeEncoderTester
      \brief Tests the SizeDeclarativeEncoder class.
   */
  class SizeDeclarativeEncoderTester : public CPPUNIT_NS::TestFixture {
    public:

    private:
      CPPUNIT_TEST_SUITE(SizeDeclarativeEncoderTester);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
