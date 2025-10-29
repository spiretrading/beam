#ifndef BEAM_PARAMETER_EVALUATOR_NODE_HPP
#define BEAM_PARAMETER_EVALUATOR_NODE_HPP
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"

namespace Beam {

  /** Base class for the ParameterEvaluatorNode. */
  class BaseParameterEvaluatorNode {
    public:
      virtual ~BaseParameterEvaluatorNode() = default;

      /** Returns the type evaluated by this node. */
      virtual std::type_index get_type() const = 0;

      /** Returns the parameter index. */
      virtual int get_index() const = 0;

      /** Initializes the parameter to use when performing an evaluation. */
      virtual void set_parameter(const void** parameter) = 0;
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

      std::type_index get_type() const override;
      int get_index() const override;
      void set_parameter(const void** parameter) override;
      Result eval() override;

    private:
      int m_index;
      const Result** m_parameter;
  };

  /**
   * Translates a ParameterExpression into a ParameterEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ParameterEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    BaseEvaluatorNode* operator ()(
        const ParameterExpression& expression) const {
      return new ParameterEvaluatorNode<T>(expression.get_index());
    }
  };

  template<typename T>
  ParameterEvaluatorNode<T>::ParameterEvaluatorNode(int index)
    : m_index(index),
      m_parameter(nullptr) {}

  template<typename T>
  std::type_index ParameterEvaluatorNode<T>::get_type() const {
    return EvaluatorNode<T>::get_type();
  }

  template<typename T>
  int ParameterEvaluatorNode<T>::get_index() const {
    return m_index;
  }

  template<typename T>
  void ParameterEvaluatorNode<T>::set_parameter(const void** parameter) {
    m_parameter = reinterpret_cast<const Result**>(parameter);
  }

  template<typename T>
  typename ParameterEvaluatorNode<T>::Result ParameterEvaluatorNode<T>::eval() {
    return **m_parameter;
  }
}

#endif
