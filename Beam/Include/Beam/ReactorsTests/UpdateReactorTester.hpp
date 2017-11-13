#ifndef BEAM_UPDATE_REACTOR_TESTER_HPP
#define BEAM_UPDATE_REACTOR_TESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ReactorsTests/ReactorTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {

  /*  \class UpdateReactorTester
      \brief Tests the UpdateReactor class.
   */
  class UpdateReactorTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests a Reactor that immediately comes to completion.
      void TestImmediateCompletion();

      //! Tests a Reactor that immediately completes with an evaluation.
      void TestCompleteWithEvaluation();

      //! Tests a Reactor that has an evaluation and then completes.
      void TestEvaluationThenCompletion();

      //! Tests a Reactor that has an evaluation, then no update then completes.
      void TestEvaluationThenNoneThenCompletion();

      //! Tests a Reactor that has an evaluation, then no update, then another
      //! no update, then completes.
      void TestEvaluationThenNoneThenNoneThenCompletion();

    private:
      CPPUNIT_TEST_SUITE(UpdateReactorTester);
        CPPUNIT_TEST(TestImmediateCompletion);
        CPPUNIT_TEST(TestCompleteWithEvaluation);
        CPPUNIT_TEST(TestEvaluationThenCompletion);
        CPPUNIT_TEST(TestEvaluationThenNoneThenCompletion);
        CPPUNIT_TEST(TestEvaluationThenNoneThenNoneThenCompletion);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
