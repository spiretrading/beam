#ifndef BEAM_GLOBAL_VARIABLE_DECLARATION_EVALUATOR_NODE_HPP
#define BEAM_GLOBAL_VARIABLE_DECLARATION_EVALUATOR_NODE_HPP
#include <memory>
#include <utility>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam::Queries {

  /**
   * Declares a global variable.
   * @param <V> The type of variable to declare.
   * @param <B> The type of body to evaluate.
   */
  template<typename V, typename B>
  class GlobalVariableDeclarationEvaluatorNode : public EvaluatorNode<B> {
    public:
      using Result = B;

      /** The type of variable to declare. */
      using Variable = V;

      /** The type of body to evaluate. */
      using Body = B;

      /**
       * Constructs a GlobalVariableDeclarationEvaluatorNode.
       * @param initialValue The variable's initial value.
       */
      GlobalVariableDeclarationEvaluatorNode(
        std::unique_ptr<EvaluatorNode<Variable>> initialValue);

      /** Returns the variable. */
      const Variable& GetVariable() const;

      /** Returns the variable. */
      Variable& GetVariable();

      /** Sets the body of the declaration. */
      void SetBody(std::unique_ptr<EvaluatorNode<Body>> body);

      Result Eval() override;

    private:
      bool m_isInitialized;
      std::unique_ptr<EvaluatorNode<Variable>> m_initialValue;
      std::unique_ptr<EvaluatorNode<Body>> m_body;
      Variable m_variable;
  };

  template<typename TypeList>
  struct GlobalVariableDeclarationEvaluatorNodeTranslator {
    template<typename Variable, typename Body>
    static std::unique_ptr<BaseEvaluatorNode> Template(
        std::unique_ptr<BaseEvaluatorNode> initialValue,
        Out<void*> address) {
      auto evaluator = std::make_unique<GlobalVariableDeclarationEvaluatorNode<
        Variable, Body>>(StaticCast<std::unique_ptr<EvaluatorNode<Variable>>>(
          std::move(initialValue)));
      *address = &evaluator->GetVariable();
      return std::move(evaluator);
    }

    template<typename T, typename U>
    struct CombineSignature {
      using type = typename boost::mpl::vector<T, U>::type;
    };

    template<typename T>
    struct MakeSignature {
      using type = typename boost::mpl::transform<TypeList,
        CombineSignature<T, boost::mpl::placeholders::_1>>::type;
    };

    using SupportedTypes = typename boost::mpl::transform<TypeList,
      MakeSignature<boost::mpl::placeholders::_1>>::type;
  };

  template<typename TypeList>
  struct GlobalVariableDeclarationEvaluatorNodeSetBody {
    template<typename Variable, typename Body>
    static void Template(BaseEvaluatorNode& declaration,
        std::unique_ptr<BaseEvaluatorNode> body) {
      static_cast<GlobalVariableDeclarationEvaluatorNode<Variable, Body>&>(
        declaration).SetBody(StaticCast<std::unique_ptr<EvaluatorNode<Body>>>(
        std::move(body)));
    }

    using SupportedTypes =
      typename GlobalVariableDeclarationEvaluatorNodeTranslator<
        TypeList>::SupportedTypes;
  };

  template<typename V, typename B>
  GlobalVariableDeclarationEvaluatorNode<V, B>::
    GlobalVariableDeclarationEvaluatorNode(
      std::unique_ptr<EvaluatorNode<Variable>> initialValue)
    : m_isInitialized(false),
      m_initialValue(std::move(initialValue)) {}

  template<typename V, typename B>
  const typename GlobalVariableDeclarationEvaluatorNode<V, B>::Variable&
      GlobalVariableDeclarationEvaluatorNode<V, B>::GetVariable() const {
    return m_variable;
  }

  template<typename V, typename B>
  typename GlobalVariableDeclarationEvaluatorNode<V, B>::Variable&
      GlobalVariableDeclarationEvaluatorNode<V, B>::GetVariable() {
    return m_variable;
  }

  template<typename V, typename B>
  void GlobalVariableDeclarationEvaluatorNode<V, B>::SetBody(
      std::unique_ptr<EvaluatorNode<Body>> body) {
    m_body = std::move(body);
  }

  BEAM_SUPPRESS_RECURSIVE_OVERFLOW()
  template<typename V, typename B>
  typename GlobalVariableDeclarationEvaluatorNode<V, B>::Result
      GlobalVariableDeclarationEvaluatorNode<V, B>::Eval() {
    if(!m_isInitialized) {
      m_isInitialized = true;
      m_variable = m_initialValue->Eval();
    }
    return m_body->Eval();
  }
  BEAM_UNSUPPRESS_RECURSIVE_OVERFLOW()
}

#endif
