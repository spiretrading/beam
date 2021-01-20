#ifndef BEAM_CONSTANT_EVALUATOR_NODE_HPP
#define BEAM_CONSTANT_EVALUATOR_NODE_HPP
#include <type_traits>
#include <utility>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /**
   * Evaluates to a constant.
   * @param <T> The type of constant to return.
   */
  template<typename T>
  class ConstantEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a ConstantEvaluatorNode.
       * @param constant The constant to evaluate to.
       */
      explicit ConstantEvaluatorNode(Result constant);

      Result Eval() override;

    private:
      Result m_constant;
  };

  template<typename T>
  ConstantEvaluatorNode<T>::ConstantEvaluatorNode(Result constant)
    : m_constant(std::move(constant)) {}

  template<typename T>
  typename ConstantEvaluatorNode<T>::Result ConstantEvaluatorNode<T>::Eval() {
    return m_constant;
  }

  template<typename TypeList>
  struct ConstantEvaluatorNodeTranslator {
    template<typename T>
    static BaseEvaluatorNode* Template(const ConstantExpression& expression) {
      return new ConstantEvaluatorNode<T>(expression.GetValue()->GetValue<T>());
    }

    using SupportedTypes = TypeList;
  };
}

#endif
