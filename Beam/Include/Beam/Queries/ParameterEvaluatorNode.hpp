#ifndef BEAM_PARAMETEREVALUATORNODE_HPP
#define BEAM_PARAMETEREVALUATORNODE_HPP
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class BaseParameterEvaluatorNode
      \brief Base class for the ParameterEvaluatorNode.
   */
  class BaseParameterEvaluatorNode {
    public:

      //! Returns the type evaluated by this node.
      virtual const std::type_info& GetResultType() const = 0;

      //! Returns the parameter index.
      virtual int GetIndex() const = 0;

      //! Initializes the parameter to use when performing an evaluation.
      virtual void SetParameter(const void** parameter) = 0;
  };

  /*! \class ParameterEvaluatorNode
      \brief Evaluates to a supplied parameter.
      \tparam ResultType The type of parameter to return.
   */
  template<typename ResultType>
  class ParameterEvaluatorNode : public EvaluatorNode<ResultType>,
      public BaseParameterEvaluatorNode {
    public:
      using Result = ResultType;

      //! Constructs a ParameterEvaluatorNode with a given index.
      /*!
        \param index The parameter's index.
      */
      ParameterEvaluatorNode(int index);

      virtual const std::type_info& GetResultType() const;

      virtual int GetIndex() const;

      virtual void SetParameter(const void** parameter);

      virtual Result Eval();

    private:
      int m_index;
      const Result** m_parameter;
  };

  template<typename TypeList>
  struct ParameterEvaluatorNodeTranslator {
    template<typename T>
    static BaseEvaluatorNode* Template(const ParameterExpression& expression) {
      return new ParameterEvaluatorNode<T>(expression.GetIndex());
    }

    using SupportedTypes = TypeList;
  };

  template<typename ResultType>
  ParameterEvaluatorNode<ResultType>::ParameterEvaluatorNode(int index)
      : m_index(index),
        m_parameter(nullptr) {}

  template<typename ResultType>
  const std::type_info& ParameterEvaluatorNode<ResultType>::
      GetResultType() const {
    return EvaluatorNode<ResultType>::GetResultType();
  }

  template<typename ResultType>
  int ParameterEvaluatorNode<ResultType>::GetIndex() const {
    return m_index;
  }

  template<typename ResultType>
  void ParameterEvaluatorNode<ResultType>::SetParameter(
      const void** parameter) {
    m_parameter = reinterpret_cast<const Result**>(parameter);
  }

  template<typename ResultType>
  typename ParameterEvaluatorNode<ResultType>::Result
      ParameterEvaluatorNode<ResultType>::Eval() {
    return **m_parameter;
  }
}
}

#endif
