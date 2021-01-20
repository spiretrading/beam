#ifndef BEAM_NOT_EVALUATOR_NODE_HPP
#define BEAM_NOT_EVALUATOR_NODE_HPP
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /** Evaluates a not expression. */
  class NotEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      /**
       * Constructs a NotEvaluatorNode.
       * @param operand The operand to evaluate.
       */
      NotEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> operand);

      bool Eval() override;

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_operand;
  };

  inline NotEvaluatorNode::NotEvaluatorNode(
    std::unique_ptr<EvaluatorNode<bool>> operand)
    : m_operand(std::move(operand)) {}

  inline bool NotEvaluatorNode::Eval() {
    return !m_operand->Eval();
  }
}

#endif
