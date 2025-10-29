#ifndef BEAM_GLOBAL_VARIABLE_DECLARATION_EVALUATOR_NODE_HPP
#define BEAM_GLOBAL_VARIABLE_DECLARATION_EVALUATOR_NODE_HPP
#include <memory>
#include <utility>
#include <boost/mp11.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam {

  /**
   * Declares a global variable.
   * @tparam V The type of variable to declare.
   * @tparam B The type of body to evaluate.
   */
  template<typename V, typename B>
  class GlobalVariableDeclarationEvaluatorNode : public EvaluatorNode<B> {
    public:

      /** The type of variable to declare. */
      using Variable = V;

      /** The type of body to evaluate. */
      using Body = B;

      using Result = B;

      /**
       * Constructs a GlobalVariableDeclarationEvaluatorNode.
       * @param initial_value The variable's initial value.
       */
      explicit GlobalVariableDeclarationEvaluatorNode(
        std::unique_ptr<EvaluatorNode<Variable>> initial_value);

      /** Returns the variable. */
      const Variable& get_variable() const;

      /** Returns the variable. */
      Variable& get_variable();

      /** Sets the body of the declaration. */
      void set_body(std::unique_ptr<EvaluatorNode<Body>> body);

      Result eval() override;

    private:
      bool m_is_initialized;
      std::unique_ptr<EvaluatorNode<Variable>> m_initial_value;
      std::unique_ptr<EvaluatorNode<Body>> m_body;
      Variable m_variable;
  };

  /**
   * Translates a GlobalVariableDeclarationExpression into a
   * GlobalVariableDeclarationEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct GlobalVariableDeclarationEvaluatorNodeTranslator {
    template<typename T, typename U>
    using combine_signature = boost::mp11::mp_list<T, U>;

    template<typename T>
    using make_signature = boost::mp11::mp_transform<combine_signature,
      boost::mp11::mp_fill<TypeList, T>, TypeList>;

    using type = boost::mp11::mp_flatten<boost::mp11::mp_transform<
      make_signature, TypeList>>;

    template<typename Variable, typename Body>
    std::unique_ptr<BaseEvaluatorNode> operator ()(
        std::unique_ptr<BaseEvaluatorNode> initial_value,
        Out<void*> address) const {
      auto evaluator = std::make_unique<GlobalVariableDeclarationEvaluatorNode<
        Variable, Body>>(static_pointer_cast<EvaluatorNode<Variable>>(
          std::move(initial_value)));
      *address = &evaluator->get_variable();
      return evaluator;
    }
  };

  /**
   * Sets the body of a GlobalVariableDeclarationEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct GlobalVariableDeclarationEvaluatorNodeSetBody {
    using type = typename GlobalVariableDeclarationEvaluatorNodeTranslator<
      TypeList>::type;

    template<typename Variable, typename Body>
    void operator ()(BaseEvaluatorNode& declaration,
        std::unique_ptr<BaseEvaluatorNode> body) const {
      static_cast<GlobalVariableDeclarationEvaluatorNode<Variable, Body>&>(
        declaration).set_body(static_pointer_cast<EvaluatorNode<Body>>(
          std::move(body)));
    }
  };

  template<typename V, typename B>
  GlobalVariableDeclarationEvaluatorNode<V, B>::
      GlobalVariableDeclarationEvaluatorNode(
        std::unique_ptr<EvaluatorNode<Variable>> initial_value)
    : m_is_initialized(false),
      m_initial_value(std::move(initial_value)) {}

  template<typename V, typename B>
  const typename GlobalVariableDeclarationEvaluatorNode<V, B>::Variable&
      GlobalVariableDeclarationEvaluatorNode<V, B>::get_variable() const {
    return m_variable;
  }

  template<typename V, typename B>
  typename GlobalVariableDeclarationEvaluatorNode<V, B>::Variable&
      GlobalVariableDeclarationEvaluatorNode<V, B>::get_variable() {
    return m_variable;
  }

  template<typename V, typename B>
  void GlobalVariableDeclarationEvaluatorNode<V, B>::set_body(
      std::unique_ptr<EvaluatorNode<Body>> body) {
    m_body = std::move(body);
  }

  BEAM_SUPPRESS_RECURSIVE_OVERFLOW()
  template<typename V, typename B>
  typename GlobalVariableDeclarationEvaluatorNode<V, B>::Result
      GlobalVariableDeclarationEvaluatorNode<V, B>::eval() {
    if(!m_is_initialized) {
      m_is_initialized = true;
      m_variable = m_initial_value->eval();
    }
    return m_body->eval();
  }
  BEAM_UNSUPPRESS_RECURSIVE_OVERFLOW()
}

#endif
