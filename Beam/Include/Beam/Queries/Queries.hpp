#ifndef BEAM_QUERIES_HPP
#define BEAM_QUERIES_HPP
#include "Beam/Pointers/ClonePtr.hpp"

namespace Beam::Queries {
  template<typename D, typename E> class AsyncDataStore;
  class BaseEvaluatorNode;
  class BaseParameterEvaluatorNode;
  template<typename T> class BasicQuery;
  template<typename D, typename E> class BufferedDataStore;
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
    class CachedDataStore;
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
    class CachedDataStoreEntry;
  template<typename ResultType> class ConstantEvaluatorNode;
  class ConstantExpression;
  class Evaluator;
  template<typename ResultType> class EvaluatorNode;
  template<typename QueryTypes> class EvaluatorTranslator;
  class ExpressionQuery;
  template<typename I, typename O, typename C> class ExpressionSubscriptions;
  class ExpressionTranslationException;
  class ExpressionVisitor;
  class FilteredQuery;
  template<typename FunctionType> class FunctionEvaluatorNode;
  class FunctionExpression;
  template<typename VariableType, typename BodyType>
    class GlobalVariableDeclarationEvaluatorNode;
  class GlobalVariableDeclarationExpression;
  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType> class IndexedExpressionSubscriptions;
  template<typename ValueType, typename IndexType,
    typename ServiceProtocolClientType> class IndexedSubscriptions;
  template<typename ValueType, typename IndexType> class IndexedValue;
  template<typename T> class IndexedQuery;
  class IndexListQuery;
  class InterruptableQuery;
  enum class InterruptionPolicy;
  template<typename QueryType, typename ValueType,
    typename EvaluatorTranslatorFilterType> class LocalDataStore;
  template<typename MemberType, typename ObjectType>
    class MemberAccessEvaluatorNode;
  class MemberAccessExpression;
  template<typename T> class NativeDataType;
  template<typename T> class NativeValue;
  class OrExpression;
  class OrEvaluatorNode;
  template<typename ResultType> class ParameterEvaluatorNode;
  class ParameterExpression;
  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
    class QueryClientPublisher;
  class QueryInterruptedException;
  template<typename T> struct QueryResult;
  class Range;
  class RangedQuery;
  template<typename t> class ReadEvaluatorNode;
  template<typename T> class ReduceEvaluatorNode;
  class ReduceExpression;
  class Sequence;
  template<typename T> class SequencedValue;
  template<typename QueryType, typename ValueType>
    class SequencedValuePublisher;
  class Sequencer;
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
    class SessionCachedDataStore;
  template<typename DataStoreType, typename EvaluatorTranslatorFilterType>
    class SessionCachedDataStoreEntry;
  class SetVariableExpression;
  class SnapshotLimit;
  class SnapshotLimitedQuery;
  template<typename C, typename V, typename I, typename T> class SqlDataStore;
  class SqlTranslator;
  template<typename ValueType, typename ServiceProtocolClientType>
    class Subscriptions;
  class TraversalExpressionVisitor;
  class TypeCompatibilityException;
  class VariableExpression;
  class VirtualDataType;
  using DataType = ClonePtr<VirtualDataType>;
  class VirtualExpression;
  using Expression = ClonePtr<VirtualExpression>;
  class VirtualValue;
  using Value = ClonePtr<VirtualValue>;
  template<typename t> class WriteEvaluatorNode;
}

#endif
