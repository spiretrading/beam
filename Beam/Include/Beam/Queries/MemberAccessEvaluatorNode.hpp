#ifndef BEAM_MEMBER_ACCESS_EVALUATOR_NODE_HPP
#define BEAM_MEMBER_ACCESS_EVALUATOR_NODE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /**
   * Returns a member/field of an object.
   * @param <M> The type of the member to return.
   * @param <T> The type of object to access.
   */
  template<typename M, typename T>
  class MemberAccessEvaluatorNode : public EvaluatorNode<M> {
    public:
      using Result = M;

      /** The type of object to access. */
      using Object = T;

      /** The type of pointer used to access the member. */
      using MemberAccessor = M Object::*;

      /**
       * Constructs a MemberAccessEvaluatorNode.
       * @param objectEvaluator The evaluator to apply the member access to.
       * @param memberAccessor The pointer used to access the member.
       */
      MemberAccessEvaluatorNode(
        std::unique_ptr<EvaluatorNode<Object>> objectEvaluator,
        MemberAccessor memberAccessor);

      Result Eval() override;

    private:
      std::unique_ptr<EvaluatorNode<Object>> m_objectEvaluator;
      MemberAccessor m_memberAccessor;
  };

  template<template<typename> class Node, typename Object,
    typename MemberAccessor>
  MemberAccessEvaluatorNode(std::unique_ptr<Node<Object>>, MemberAccessor) ->
    MemberAccessEvaluatorNode<std::decay_t<decltype(
      std::declval<Object>().*std::declval<MemberAccessor>())>, Object>;

  template<typename M, typename T>
  MemberAccessEvaluatorNode<M, T>::MemberAccessEvaluatorNode(
    std::unique_ptr<EvaluatorNode<Object>> objectEvaluator,
    MemberAccessor memberAccessor)
    : m_objectEvaluator(std::move(objectEvaluator)),
      m_memberAccessor(memberAccessor) {}

  template<typename M, typename T>
  typename MemberAccessEvaluatorNode<M, T>::Result
      MemberAccessEvaluatorNode<M, T>::Eval() {
    return m_objectEvaluator->Eval().*m_memberAccessor;
  }
}

#endif
