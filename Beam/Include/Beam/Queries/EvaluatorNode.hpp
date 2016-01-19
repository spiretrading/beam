#ifndef BEAM_EVALUATORNODE_HPP
#define BEAM_EVALUATORNODE_HPP
#include <typeinfo>
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class BaseEvaluatorNode
      \brief Base class for an EvaluatorNode.
   */
  class BaseEvaluatorNode {
    public:
      virtual ~BaseEvaluatorNode() = default;

      //! Returns the type evaluated by this node.
      virtual const std::type_info& GetResultType() const = 0;
 };

  /*! \class EvaluatorNode
      \brief Evaluates an Expression.
      \tparam ResultType The type of the result.
   */
  template<typename ResultType>
  class EvaluatorNode : public BaseEvaluatorNode {
    public:

      //! The result of an evaluation.
      using Result = ResultType;

      virtual ~EvaluatorNode() = default;

      //! Returns the type evaluated by this node.
      virtual const std::type_info& GetResultType() const;

      //! Evaluates the expression.
      virtual Result Eval() = 0;
  };

  template<typename ResultType>
  const std::type_info& EvaluatorNode<ResultType>::GetResultType() const {
    return typeid(Result);
  }
}
}

#endif
