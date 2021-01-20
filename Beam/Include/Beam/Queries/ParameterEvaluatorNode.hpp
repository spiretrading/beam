#ifndef BEAM_PARAMETER_EVALUATOR_NODE_HPP
#define BEAM_PARAMETER_EVALUATOR_NODE_HPP
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /** Base class for the ParameterEvaluatorNode. */
  class BaseParameterEvaluatorNode {
    public:
      virtual ~BaseParameterEvaluatorNode() = default;

      /** Returns the type evaluated by this node. */
      virtual const std::type_info& GetResultType() const = 0;

      /** Returns the parameter index. */
      virtual int GetIndex() const = 0;

      /** Initializes the parameter to use when performing an evaluation. */
      virtual void SetParameter(const void** parameter) = 0;
  };

  /**
   * Evaluates to a supplied parameter.
   * @param T The type of parameter to return.
   */
  template<typename T>
  class ParameterEvaluatorNode :
      public EvaluatorNode<T>, public BaseParameterEvaluatorNode {
    public:
      using Result = T;

      /**
       * Constructs a ParameterEvaluatorNode with a given index.
       * @param index The parameter's index.
       */
      explicit ParameterEvaluatorNode(int index);

      const std::type_info& GetResultType() const override;

      int GetIndex() const override;

      void SetParameter(const void** parameter) override;

      Result Eval() override;

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

  template<typename T>
  ParameterEvaluatorNode<T>::ParameterEvaluatorNode(int index)
    : m_index(index),
      m_parameter(nullptr) {}

  template<typename T>
  const std::type_info& ParameterEvaluatorNode<T>::GetResultType() const {
    return EvaluatorNode<T>::GetResultType();
  }

  template<typename T>
  int ParameterEvaluatorNode<T>::GetIndex() const {
    return m_index;
  }

  template<typename T>
  void ParameterEvaluatorNode<T>::SetParameter(const void** parameter) {
    m_parameter = reinterpret_cast<const Result**>(parameter);
  }

  template<typename T>
  typename ParameterEvaluatorNode<T>::Result ParameterEvaluatorNode<T>::Eval() {
    return **m_parameter;
  }
}

#endif
