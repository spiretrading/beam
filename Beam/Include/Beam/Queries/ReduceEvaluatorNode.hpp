#ifndef BEAM_REDUCEEVALUATORNODE_HPP
#define BEAM_REDUCEEVALUATORNODE_HPP
#include <memory>
#include <utility>
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class ReduceEvaluatorNode
      \brief Evaluates a ReduceExpression.
      \tparam T The type being reduced.
   */
  template<typename T>
  class ReduceEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = typename EvaluatorNode<T>::Result;

      //! Constructs a ReduceEvaluatorNode.
      /*!
        \param reducer The Evaluator used to perform the reduction.
        \param series The series to reduce.
        \param initialValue The initial value.
      */
      ReduceEvaluatorNode(std::unique_ptr<Evaluator> reducer,
        std::unique_ptr<EvaluatorNode<T>> series, const Result& initialValue);

      virtual Result Eval();

    private:
      std::unique_ptr<Evaluator> m_reducer;
      std::unique_ptr<EvaluatorNode<T>> m_series;
      Result m_value;
  };

  template<template<typename> class Node, typename T>
  ReduceEvaluatorNode(std::unique_ptr<Evaluator>, std::unique_ptr<Node<T>>,
    const typename EvaluatorNode<T>::Result&) -> ReduceEvaluatorNode<T>;

  template<typename T>
  ReduceEvaluatorNode<T>::ReduceEvaluatorNode(
    std::unique_ptr<Evaluator> reducer,
    std::unique_ptr<EvaluatorNode<T>> series, const Result& initialValue)
    : m_reducer(std::move(reducer)),
      m_series(std::move(series)),
      m_value(initialValue) {}
}
}

#endif
