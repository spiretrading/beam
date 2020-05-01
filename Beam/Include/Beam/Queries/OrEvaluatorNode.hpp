#ifndef BEAM_OREVALUATORNODE_HPP
#define BEAM_OREVALUATORNODE_HPP
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class OrEvaluatorNode
      \brief Evaluates an or expression.
   */
  class OrEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      //! Constructs an OrEvaluatorNode.
      /*!
        \param left The left hand side to evaluate.
        \param right The right hand side to evaluate.
      */
      OrEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> left,
        std::unique_ptr<EvaluatorNode<bool>> right);

      virtual bool Eval();

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_left;
      std::unique_ptr<EvaluatorNode<bool>> m_right;
  };

  inline OrEvaluatorNode::OrEvaluatorNode(
      std::unique_ptr<EvaluatorNode<bool>> left,
      std::unique_ptr<EvaluatorNode<bool>> right)
      : m_left(std::move(left)),
        m_right(std::move(right)) {}

  inline bool OrEvaluatorNode::Eval() {
    return m_left->Eval() || m_right->Eval();
  }
}
}

#endif
