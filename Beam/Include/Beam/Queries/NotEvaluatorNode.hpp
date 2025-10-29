#ifndef BEAM_NOT_EVALUATOR_NODE_HPP
#define BEAM_NOT_EVALUATOR_NODE_HPP
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"

namespace Beam {

  /** Evaluates a not expression. */
  class NotEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      /**
       * Constructs a NotEvaluatorNode.
       * @param operand The operand to evaluate.
       */
      explicit NotEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> operand);

      bool eval() override;

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_operand;
  };

  inline NotEvaluatorNode::NotEvaluatorNode(
    std::unique_ptr<EvaluatorNode<bool>> operand)
    : m_operand(std::move(operand)) {}

  inline bool NotEvaluatorNode::eval() {
    return !m_operand->eval();
  }
}

#endif
