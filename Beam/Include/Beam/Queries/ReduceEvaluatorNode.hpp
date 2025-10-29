#ifndef BEAM_REDUCE_EVALUATOR_NODE_HPP
#define BEAM_REDUCE_EVALUATOR_NODE_HPP
#include <memory>
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam {
  class Evaluator;

  /**
   * Evaluates a ReduceExpression.
   * @tparam T The type being reduced.
   */
  template<typename T>
  class ReduceEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = typename EvaluatorNode<T>::Result;

      /**
       * Constructs a ReduceEvaluatorNode.
       * @param reducer The Evaluator used to perform the reduction.
       * @param series The series to reduce.
       * @param initial_value The initial value.
       */
      ReduceEvaluatorNode(std::unique_ptr<Evaluator> reducer,
        std::unique_ptr<EvaluatorNode<T>> series, Result initial_value);

      Result eval() override;

    private:
      std::unique_ptr<Evaluator> m_reducer;
      std::unique_ptr<EvaluatorNode<T>> m_series;
      Result m_value;
  };

  template<template<typename> class Node, typename T>
  ReduceEvaluatorNode(std::unique_ptr<Evaluator>, std::unique_ptr<Node<T>>,
    const typename EvaluatorNode<T>::Result&) -> ReduceEvaluatorNode<T>;

  /**
   * Translates a ReduceExpression into a ReduceEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ReduceEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    BaseEvaluatorNode* operator ()(std::unique_ptr<Evaluator> reducer,
        std::unique_ptr<BaseEvaluatorNode> series,
        const Value& initial_value) const {
      return new ReduceEvaluatorNode(
        std::move(reducer), static_pointer_cast<EvaluatorNode<T>>(
          std::move(series)), initial_value.as<T>());
    }
  };

  template<typename T>
  ReduceEvaluatorNode<T>::ReduceEvaluatorNode(
    std::unique_ptr<Evaluator> reducer,
    std::unique_ptr<EvaluatorNode<T>> series, Result initial_value)
    : m_reducer(std::move(reducer)),
      m_series(std::move(series)),
      m_value(std::move(initial_value)) {}
}

#endif
