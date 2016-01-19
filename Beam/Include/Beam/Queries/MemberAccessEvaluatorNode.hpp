#ifndef BEAM_MEMBERACCESSEVALUATORNODE_HPP
#define BEAM_MEMBERACCESSEVALUATORNODE_HPP
#include <memory>
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class MemberAccessEvaluatorNode
      \brief Returns a member/field of an object.
      \tparam MemberType The type of the member to return.
      \tparam ObjectType The type of object to access.
   */
  template<typename MemberType, typename ObjectType>
  class MemberAccessEvaluatorNode : public EvaluatorNode<MemberType> {
    public:
      typedef MemberType Result;

      //! The type of object to access.
      typedef ObjectType Object;

      //! The type of pointer used to access the member.
      typedef MemberType Object::* MemberAccessor;

      //! Constructs a MemberAccessEvaluatorNode.
      /*!
        \param objectEvaluator The evaluator to apply the member access to.
        \param memberAccessor The pointer used to access the member.
      */
      MemberAccessEvaluatorNode(
        std::unique_ptr<EvaluatorNode<Object>> objectEvaluator,
        MemberAccessor memberAccessor);

      virtual Result Eval();

    private:
      std::unique_ptr<EvaluatorNode<Object>> m_objectEvaluator;
      MemberAccessor m_memberAccessor;
  };

  template<typename MemberType, typename ObjectType>
  MemberAccessEvaluatorNode<MemberType, ObjectType>::MemberAccessEvaluatorNode(
      std::unique_ptr<EvaluatorNode<Object>> objectEvaluator,
      MemberAccessor memberAccessor)
      : m_objectEvaluator(std::move(objectEvaluator)),
        m_memberAccessor(memberAccessor) {}

  template<typename MemberType, typename ObjectType>
  typename MemberAccessEvaluatorNode<MemberType, ObjectType>::Result
      MemberAccessEvaluatorNode<MemberType, ObjectType>::Eval() {
    return m_objectEvaluator->Eval().*m_memberAccessor;
  }
}
}

#endif
