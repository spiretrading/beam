#ifndef BEAM_EVALUATOR_NODE_HPP
#define BEAM_EVALUATOR_NODE_HPP
#include <typeindex>

namespace Beam {

  /** Base class for an EvaluatorNode. */
  class BaseEvaluatorNode {
    public:
      virtual ~BaseEvaluatorNode() = default;

      /** Returns the type evaluated by this node. */
      virtual std::type_index get_type() const = 0;
 };

  /**
   * Evaluates an Expression.
   * @param R The type of the result.
   */
  template<typename R>
  class EvaluatorNode : public BaseEvaluatorNode {
    public:

      /** The result of an evaluation. */
      using Result = R;

      /** Evaluates the expression. */
      virtual Result eval() = 0;

      std::type_index get_type() const override;
  };

  template<typename R>
  std::type_index EvaluatorNode<R>::get_type() const {
    return std::type_index(typeid(Result));
  }
}

#endif
