#ifndef BEAM_QUERIES_TESTS_HPP
#define BEAM_QUERIES_TESTS_HPP
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries::Tests {
  class BufferedDataStoreTester;
  class CachedDataStoreTester;
  class ConstantEvaluatorNodeTester;
  class ConstantExpressionTester;
  class ConversionEvaluatorNodeTester;
  class EvaluatorTester;
  class ExpressionSubscriptionsTester;
  class FilteredQueryTester;
  class FunctionEvaluatorNodeTester;
  class FunctionExpressionTester;
  class IndexedQueryTester;
  class IndexedValueTester;
  class LocalDataStoreTester;
  class MemberAccessEvaluatorNodeTester;
  class MemberAccessExpressionTester;
  class NativeDataTypeTester;
  class NativeValueTester;
  class OrEvaluatorNodeTester;
  class OrExpressionTester;
  class ParameterEvaluatorNodeTester;
  class ParameterExpressionTester;
  class RangeTester;
  class RangedQueryTester;
  class ReduceEvaluatorNodeTester;
  class ReduceExpressionTester;
  class SequenceTester;
  class SequencedValueTester;
  class SequencedValuePublisherTester;
  class SequencerTester;
  class SessionCachedDataStoreTester;
  class SnapshotLimitTester;
  class SnapshotLimitedQueryTester;
  class SqlDataStoreTester;
  class SqlTranslatorTester;
  class SubscriptionsTester;
  template<typename Q, typename V> class TestDataStore;
  class TestDataStoreTester;
}

#endif
