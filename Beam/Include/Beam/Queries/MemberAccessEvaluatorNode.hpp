#ifndef BEAM_MEMBER_ACCESS_EVALUATOR_NODE_HPP
#define BEAM_MEMBER_ACCESS_EVALUATOR_NODE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"

namespace Beam {

  /**
   * Returns a member/field of an object.
   * @tparam T The type of object to access.
   * @tparam M The type of the member to return.
   */
  template<typename T, typename M>
  class MemberAccessEvaluatorNode : public EvaluatorNode<M> {
    public:

      /** The type of object to access. */
      using Object = T;

      /** The type of pointer used to access the member. */
      using MemberAccessor = M Object::*;

      using Result = M;

      /**
       * Constructs a MemberAccessEvaluatorNode.
       * @param evaluator The evaluator to apply the member access to.
       * @param accessor The pointer used to access the member.
       */
      MemberAccessEvaluatorNode(
        std::unique_ptr<EvaluatorNode<Object>> evaluator,
        MemberAccessor accessor);

      Result eval() override;

    private:
      std::unique_ptr<EvaluatorNode<Object>> m_evaluator;
      MemberAccessor m_accessor;
  };

  template<template<typename> class Node, typename Object,
    typename MemberAccessor>
  MemberAccessEvaluatorNode(std::unique_ptr<Node<Object>>, MemberAccessor) ->
    MemberAccessEvaluatorNode<Object, std::remove_cvref_t<decltype(
      std::declval<Object>().*std::declval<MemberAccessor>())>>;

  template<typename T, typename M>
  MemberAccessEvaluatorNode<T, M>::MemberAccessEvaluatorNode(
    std::unique_ptr<EvaluatorNode<Object>> evaluator, MemberAccessor accessor)
    : m_evaluator(std::move(evaluator)),
      m_accessor(accessor) {}

  template<typename T, typename M>
  typename MemberAccessEvaluatorNode<T, M>::Result
      MemberAccessEvaluatorNode<T, M>::eval() {
    return m_evaluator->eval().*m_accessor;
  }
}

#endif
