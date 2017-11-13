#ifndef BEAM_LAST_REACTOR_TESTER_HPP
#define BEAM_LAST_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class LastReactorTester
      \brief Tests the Last Reactor.
   */
  class LastReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests immediately coming to completion.
      void TestImmediateCompletion();

      //! Tests immediately coming to completion with a value.
      void TestCompleteWithEvaluation();

      //! Tests an evaluation followed by a completion with evaluation.
      void TestEvaluationThenCompleteWithEvaluation();

      //! Tests an evaluation followed by an evaluation followed by completion.
      void TestEvaluationThenEvaluationThenComplete();

    private:
      CPPUNIT_TEST_SUITE(LastReactorTester);
        CPPUNIT_TEST(TestImmediateCompletion);
        CPPUNIT_TEST(TestCompleteWithEvaluation);
        CPPUNIT_TEST(TestEvaluationThenCompleteWithEvaluation);
        CPPUNIT_TEST(TestEvaluationThenEvaluationThenComplete);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
