#ifndef BEAM_CONSTANTEVALUATORNODE_HPP
#define BEAM_CONSTANTEVALUATORNODE_HPP
#include <type_traits>
#include <utility>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class ConstantEvaluatorNode
      \brief Evaluates to a constant.
      \tparam ResultType The type of constant to return.
   */
  template<typename ResultType>
  class ConstantEvaluatorNode : public EvaluatorNode<ResultType> {
    public:
      typedef ResultType Result;

      //! Constructs a ConstantEvaluatorNode.
      /*!
        \param constant The constant to evaluate to.
      */
      ConstantEvaluatorNode(const Result& constant);

      virtual Result Eval();

    private:
      Result m_constant;
  };

  //! Makes a ConstantEvaluatorNode that wraps a value.
  /*!
    \param value The value to wrap.
  */
  template<typename T>
  ConstantEvaluatorNode<typename std::decay<T>::type> MakeConstantEvaluatorNode(
      T&& value) {
    return ConstantEvaluatorNode<typename std::decay<T>::type>(
      std::forward<T>(value));
  }

  template<typename ResultType>
  ConstantEvaluatorNode<ResultType>::ConstantEvaluatorNode(
      const Result& constant)
      : m_constant(constant) {}

  template<typename ResultType>
  typename ConstantEvaluatorNode<ResultType>::Result
      ConstantEvaluatorNode<ResultType>::Eval() {
    return m_constant;
  }

  template<typename TypeList>
  struct ConstantEvaluatorNodeTranslator {
    template<typename T>
    static BaseEvaluatorNode* Template(const ConstantExpression& expression) {
      return new ConstantEvaluatorNode<T>(expression.GetValue()->GetValue<T>());
    }

    typedef TypeList SupportedTypes;
  };
}
}

#endif
