#ifndef BEAM_CONSTANT_EVALUATOR_NODE_HPP
#define BEAM_CONSTANT_EVALUATOR_NODE_HPP
#include <type_traits>
#include <utility>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"

namespace Beam {

  /**
   * Evaluates to a constant.
   * @tparam T The type of constant to return.
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

      Result eval() override;

    private:
      Result m_constant;
  };

  /**
   * Translates a ConstantExpression into a ConstantEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ConstantEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    BaseEvaluatorNode* operator ()(const ConstantExpression& expression) const {
      return new ConstantEvaluatorNode(expression.get_value().as<T>());
    }
  };

  template<typename T>
  ConstantEvaluatorNode<T>::ConstantEvaluatorNode(Result constant)
    : m_constant(std::move(constant)) {}

  template<typename T>
  typename ConstantEvaluatorNode<T>::Result ConstantEvaluatorNode<T>::eval() {
    return m_constant;
  }
}

#endif
