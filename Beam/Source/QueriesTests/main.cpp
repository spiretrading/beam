#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "Beam/QueriesTests/AsyncDataStoreTester.hpp"
#include "Beam/QueriesTests/BufferedDataStoreTester.hpp"
#include "Beam/QueriesTests/CachedDataStoreTester.hpp"
#include "Beam/QueriesTests/ConstantEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/ConstantExpressionTester.hpp"
#include "Beam/QueriesTests/ConversionEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/EvaluatorTester.hpp"
#include "Beam/QueriesTests/ExpressionSubscriptionsTester.hpp"
#include "Beam/QueriesTests/FilteredQueryTester.hpp"
#include "Beam/QueriesTests/FunctionEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/FunctionExpressionTester.hpp"
#include "Beam/QueriesTests/IndexedQueryTester.hpp"
#include "Beam/QueriesTests/IndexedValueTester.hpp"
#include "Beam/QueriesTests/InterruptableQueryTester.hpp"
#include "Beam/QueriesTests/LocalDataStoreTester.hpp"
#include "Beam/QueriesTests/MemberAccessExpressionTester.hpp"
#include "Beam/QueriesTests/MemberAccessEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/NativeDataTypeTester.hpp"
#include "Beam/QueriesTests/NativeValueTester.hpp"
#include "Beam/QueriesTests/OrEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/OrExpressionTester.hpp"
#include "Beam/QueriesTests/ParameterEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/ParameterExpressionTester.hpp"
#include "Beam/QueriesTests/RangeTester.hpp"
#include "Beam/QueriesTests/RangedQueryTester.hpp"
#include "Beam/QueriesTests/ReduceEvaluatorNodeTester.hpp"
#include "Beam/QueriesTests/ReduceExpressionTester.hpp"
#include "Beam/QueriesTests/SequenceTester.hpp"
#include "Beam/QueriesTests/SequencerTester.hpp"
#include "Beam/QueriesTests/SequencedValueTester.hpp"
#include "Beam/QueriesTests/SequencedValuePublisherTester.hpp"
#include "Beam/QueriesTests/SessionCachedDataStoreTester.hpp"
#include "Beam/QueriesTests/SnapshotLimitTester.hpp"
#include "Beam/QueriesTests/SnapshotLimitedQueryTester.hpp"
#include "Beam/QueriesTests/SqlDataStoreTester.hpp"
#include "Beam/QueriesTests/SqlTranslatorTester.hpp"
#include "Beam/QueriesTests/SubscriptionsTester.hpp"
#include "Beam/QueriesTests/TestDataStoreTester.hpp"

using namespace Beam::Queries::Tests;

int main() {
  CPPUNIT_NS::TextUi::TestRunner runner;
  CppUnit::BriefTestProgressListener listener;
  runner.addTest(SequenceTester::suite());
  runner.addTest(SequencedValueTester::suite());
  runner.addTest(IndexedValueTester::suite());
  runner.addTest(SnapshotLimitTester::suite());
  runner.addTest(RangeTester::suite());
  runner.addTest(RangedQueryTester::suite());
  runner.addTest(SnapshotLimitedQueryTester::suite());
  runner.addTest(InterruptableQueryTester::suite());
  runner.addTest(IndexedQueryTester::suite());
  runner.addTest(NativeDataTypeTester::suite());
  runner.addTest(NativeValueTester::suite());
  runner.addTest(ConstantExpressionTester::suite());
  runner.addTest(FilteredQueryTester::suite());
  runner.addTest(ConstantEvaluatorNodeTester::suite());
  runner.addTest(ParameterExpressionTester::suite());
  runner.addTest(ParameterEvaluatorNodeTester::suite());
  runner.addTest(EvaluatorTester::suite());
  runner.addTest(FunctionExpressionTester::suite());
  runner.addTest(FunctionEvaluatorNodeTester::suite());
  runner.addTest(ConversionEvaluatorNodeTester::suite());
  runner.addTest(MemberAccessExpressionTester::suite());
  runner.addTest(MemberAccessEvaluatorNodeTester::suite());
  runner.addTest(OrExpressionTester::suite());
  runner.addTest(OrEvaluatorNodeTester::suite());
  runner.addTest(ReduceExpressionTester::suite());
  runner.addTest(ReduceEvaluatorNodeTester::suite());
  runner.addTest(SequencedValuePublisherTester::suite());
  runner.addTest(SubscriptionsTester::suite());
  runner.addTest(TestDataStoreTester::suite());
  runner.addTest(LocalDataStoreTester::suite());
  runner.addTest(BufferedDataStoreTester::suite());
  runner.addTest(CachedDataStoreTester::suite());
  runner.addTest(SessionCachedDataStoreTester::suite());
  runner.addTest(AsyncDataStoreTester::suite());
  runner.addTest(ExpressionSubscriptionsTester::suite());
  runner.addTest(SqlDataStoreTester::suite());
  runner.addTest(SqlTranslatorTester::suite());
  runner.addTest(SequencerTester::suite());
  runner.eventManager().addListener(&listener);
  runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(),
    CPPUNIT_NS::stdCOut()));
  auto wasSucessful = runner.run();
  return wasSucessful ? 0 : -1;
}
