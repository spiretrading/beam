#ifndef BEAM_QUERIES_HPP
#define BEAM_QUERIES_HPP
#include "Beam/Pointers/ClonePtr.hpp"

namespace Beam::Queries {
  class AndExpression;
  class AndEvaluatorNode;
  template<typename D, typename E> class AsyncDataStore;
  class BaseEvaluatorNode;
  class BaseParameterEvaluatorNode;
  template<typename T> class BasicQuery;
  template<typename D, typename E> class BufferedDataStore;
  template<typename D, typename T> class CachedDataStore;
  template<typename D, typename T> class CachedDataStoreEntry;
  template<typename T> class ConstantEvaluatorNode;
  class ConstantExpression;
  class Evaluator;
  template<typename T> class EvaluatorNode;
  template<typename Q> class EvaluatorTranslator;
  class ExpressionQuery;
  template<typename I, typename O, typename C> class ExpressionSubscriptions;
  class ExpressionTranslationException;
  class ExpressionVisitor;
  class FilteredQuery;
  template<typename F> class FunctionEvaluatorNode;
  class FunctionExpression;
  template<typename V, typename B> class GlobalVariableDeclarationEvaluatorNode;
  class GlobalVariableDeclarationExpression;
  template<typename T, typename O, typename I, typename C>
    class IndexedExpressionSubscriptions;
  template<typename V, typename I, typename C> class IndexedSubscriptions;
  template<typename V, typename I> class IndexedValue;
  template<typename T> class IndexedQuery;
  class IndexListQuery;
  class InterruptableQuery;
  enum class InterruptionPolicy;
  template<typename Q, typename V, typename T> class LocalDataStore;
  template<typename M, typename T> class MemberAccessEvaluatorNode;
  class MemberAccessExpression;
  template<typename T> class NativeDataType;
  template<typename T> class NativeValue;
  class NotExpression;
  class NotEvaluatorNode;
  class OrEvaluatorNode;
  class OrExpression;
  template<typename I, typename A> class PagedQuery;
  template<typename T> class ParameterEvaluatorNode;
  class ParameterExpression;
  template<typename V, typename Q, typename E, typename C, typename S,
    typename M> class QueryClientPublisher;
  class QueryInterruptedException;
  template<typename T> struct QueryResult;
  class Range;
  class RangedQuery;
  template<typename T> class ReadEvaluatorNode;
  template<typename T> class ReduceEvaluatorNode;
  class ReduceExpression;
  class Sequence;
  template<typename T> class SequencedValue;
  template<typename Q, typename V> class SequencedValuePublisher;
  class Sequencer;
  template<typename D, typename T> class SessionCachedDataStore;
  template<typename D, typename T> class SessionCachedDataStoreEntry;
  class SetVariableExpression;
  class SnapshotLimit;
  class SnapshotLimitedQuery;
  template<typename C, typename V, typename I, typename T> class SqlDataStore;
  class SqlTranslator;
  template<typename V, typename C> class Subscriptions;
  class TraversalExpressionVisitor;
  class TypeCompatibilityException;
  class VariableExpression;
  class VirtualDataType;
  using DataType = ClonePtr<VirtualDataType>;
  class VirtualExpression;
  using Expression = ClonePtr<VirtualExpression>;
  class VirtualValue;
  using Value = ClonePtr<VirtualValue>;
  template<typename T> class WriteEvaluatorNode;
}

#endif
