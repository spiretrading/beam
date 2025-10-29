#ifndef BEAM_AND_EVALUATOR_NODE_HPP
#define BEAM_AND_EVALUATOR_NODE_HPP
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"

namespace Beam {

  /** Evaluates an and expression. */
  class AndEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      /**
       * Constructs an AndEvaluatorNode.
       * @param left The left hand side to evaluate.
       * @param right The right hand side to evaluate.
       */
      AndEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> left,
        std::unique_ptr<EvaluatorNode<bool>> right);

      bool eval() override;

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_left;
      std::unique_ptr<EvaluatorNode<bool>> m_right;
  };

  inline AndEvaluatorNode::AndEvaluatorNode(
    std::unique_ptr<EvaluatorNode<bool>> left,
    std::unique_ptr<EvaluatorNode<bool>> right)
    : m_left(std::move(left)),
      m_right(std::move(right)) {}

  inline bool AndEvaluatorNode::eval() {
    return m_left->eval() && m_right->eval();
  }
}

#endif
