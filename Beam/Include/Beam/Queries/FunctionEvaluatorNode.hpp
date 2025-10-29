#ifndef BEAM_FUNCTION_EVALUATOR_NODE_HPP
#define BEAM_FUNCTION_EVALUATOR_NODE_HPP
#include <memory>
#include <tuple>
#include <boost/callable_traits.hpp>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam {
namespace Details {
  template<typename F>
  struct function_parameter_tuple;

  template<typename R, typename... Args>
  struct function_parameter_tuple<R(Args...)> {
    using type =
      std::tuple<std::unique_ptr<EvaluatorNode<std::remove_cvref_t<Args>>>...>;
  };

  template<typename F>
  using function_parameter_tuple_t = typename function_parameter_tuple<F>::type;
}

  /**
   * Evaluates a function.
   * @tparam F The type of function to evaluate.
   */
  template<typename F>
  class FunctionEvaluatorNode : public EvaluatorNode<
      typename boost::callable_traits::return_type_t<F>> {
    public:

      /** The type of function to evaluate. */
      using Function = F;

      using Result = typename EvaluatorNode<
        typename boost::callable_traits::return_type_t<Function>>::Result;

      /**
       * Constructs a FunctionEvaluatorNode.
       * @param function The function to evaluate.
       * @param args The arguments to pass to the <i>function</i>.
       */
      template<typename... Args>
      explicit FunctionEvaluatorNode(
        Function function, std::unique_ptr<Args>... args);

      /**
       * Constructs a FunctionEvaluatorNode.
       * @param function The function to evaluator.
       * @param args The arguments to pass to the <i>function</i>.
       */
      FunctionEvaluatorNode(Function function,
        std::vector<std::unique_ptr<BaseEvaluatorNode>> args);

      Result eval() override;

    private:
      Function m_function;
      Details::function_parameter_tuple_t<
        boost::callable_traits::function_type_t<Function>> m_arguments;
  };

  /**
   * Makes a FunctionEvaluatorNode.
   * @param function The function to evaluate.
   * @param args The parameters to pass to the <i>function</i>.
   */
  template<typename Function, typename... Args>
  std::unique_ptr<FunctionEvaluatorNode<Function>> make_function_evaluator_node(
      Function function, std::unique_ptr<Args>... args) {
    return std::make_unique<FunctionEvaluatorNode<Function>>(
      function, std::move(args)...);
  }

  /**
   * Translates a FunctionExpression into a FunctionEvaluatorNode.
   * @tparam F The type of function to evaluate.
   */
  template<typename F>
  struct FunctionEvaluatorNodeTranslator {
    using type = typename F::type;

    template<typename... Args>
    BaseEvaluatorNode* operator ()(
        std::vector<std::unique_ptr<BaseEvaluatorNode>> parameters) const {
      using Operation = typename F::template Operation<Args...>;
      return new FunctionEvaluatorNode<Operation>(
        Operation(), std::move(parameters));
    };
  };

  template<typename F>
  template<typename... Args>
  FunctionEvaluatorNode<F>::FunctionEvaluatorNode(
    Function function, std::unique_ptr<Args>... args)
    : m_function(std::move(function)),
      m_arguments(std::move(args)...) {}

  template<typename F>
  FunctionEvaluatorNode<F>::FunctionEvaluatorNode(
      Function function, std::vector<std::unique_ptr<BaseEvaluatorNode>> args)
      : m_function(std::move(function)) {
    if(args.size() != std::tuple_size_v<decltype(m_arguments)>) {
      boost::throw_with_location(
        std::invalid_argument("args has the wrong size."));
    }
    std::apply([&] (auto&... arg) {
      auto index = 0;
      ((arg = static_pointer_cast<std::remove_cvref_t<decltype(*arg)>>(
        std::move(args[index++]))), ...);
    }, m_arguments);
  }

  template<typename F>
  typename FunctionEvaluatorNode<F>::Result FunctionEvaluatorNode<F>::eval() {
    return std::apply([&] (auto&... arg) {
      return m_function(arg->eval()...);
    }, m_arguments);
  }
}

#endif
