#ifndef BEAM_EVALUATOR_NODE_HPP
#define BEAM_EVALUATOR_NODE_HPP
#include <typeinfo>
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /** Base class for an EvaluatorNode. */
  class BaseEvaluatorNode {
    public:
      virtual ~BaseEvaluatorNode() = default;

      /** Returns the type evaluated by this node. */
      virtual const std::type_info& GetResultType() const = 0;
 };

  /**
   * Evaluates an Expression.
   * @param R The type of the result.
   */
  template<typename T>
  class EvaluatorNode : public BaseEvaluatorNode {
    public:

      /** The result of an evaluation. */
      using Result = T;

      /** Returns the type evaluated by this node. */
      const std::type_info& GetResultType() const override;

      /** Evaluates the expression. */
      virtual Result Eval() = 0;
  };

  template<typename T>
  const std::type_info& EvaluatorNode<T>::GetResultType() const {
    return typeid(Result);
  }
}

#endif
