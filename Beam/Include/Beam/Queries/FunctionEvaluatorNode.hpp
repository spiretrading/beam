#ifndef BEAM_FUNCTION_EVALUATOR_NODE_HPP
#define BEAM_FUNCTION_EVALUATOR_NODE_HPP
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Utilities/Casts.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam::Queries {
namespace Details {
  template<typename Type, typename Tuple>
  struct AppendTuple {};

  template<typename Type, typename... Args>
  struct AppendTuple<Type, std::tuple<Args...>> {
    using type = std::tuple<Args..., std::unique_ptr<EvaluatorNode<Type>>>;
  };

  template<typename Type, typename Container, typename Enabled = void>
  struct ExpandParameters {};

  template<typename Type, typename Container>
  struct ExpandParameters<Type, Container,
      typename std::enable_if<boost::mpl::empty<Container>::value>::type> {
    using type = Type;
  };

template<typename Type, typename Container>
  struct ExpandParameters<Type, Container,
      std::enable_if_t<!boost::mpl::empty<Container>::value>> {
    using Tuple = typename AppendTuple<
      std::decay_t<typename boost::mpl::front<Container>::type>, Type>::type;
    using type = typename ExpandParameters<Tuple, std::decay_t<
      typename boost::mpl::pop_front<Container>::type>>::type;
  };

  template<typename Function>
  struct FunctionParameterTuple {
    using Signature = typename GetSignature<Function>::type;
    using Parameters = typename boost::function_types::parameter_types<
      Signature>::type;
    using type = typename ExpandParameters<std::tuple<>, Parameters>::type;
  };

  struct AssignParameters {
    mutable int m_index;
    std::vector<std::unique_ptr<BaseEvaluatorNode>>* m_args;

    AssignParameters(std::vector<std::unique_ptr<BaseEvaluatorNode>>& args)
      : m_index(0),
        m_args(&args) {}

    template<typename T>
    void operator ()(std::unique_ptr<EvaluatorNode<T>>& parameter) const {
      parameter = StaticCast<std::unique_ptr<EvaluatorNode<T>>>(
        std::move((*m_args)[m_index]));
      ++m_index;
    }
  };
}

  /**
   * Evaluates a function.
   * @param <F> The type of function to evaluate.
   */
  template<typename F>
  class FunctionEvaluatorNode : public EvaluatorNode<
      typename boost::function_types::result_type<
        typename GetSignature<F>::type>::type> {
    public:
      using Result = typename EvaluatorNode<
        typename boost::function_types::result_type<
        typename GetSignature<F>::type>::type>::Result;

      /** The type of function called. */
      using Function = F;

      /**
       * Constructs a FunctionEvaluatorNode.
       * @param function The function to evaluate.
       * @param args The parameters to pass to the <i>function</i>.
       */
      template<typename... Args>
      FunctionEvaluatorNode(const Function& function,
        std::unique_ptr<Args>... args)
        : m_invoker(function),
          m_parameters(std::move(args)...) {}

      /**
       * Constructs a FunctionEvaluatorNode.
       * @param function The function to evaluator.
       * @param args The parameters to pass to the <i>function</i>.
       */
      FunctionEvaluatorNode(const Function& function,
        std::vector<std::unique_ptr<BaseEvaluatorNode>> args);

      Result Eval() override;

    private:
      struct Invoker {
        Function m_function;

        Invoker(const Function& function);

        template<typename... Args>
        Result operator ()(Args&... args) const;
      };
      Invoker m_invoker;
      typename Details::FunctionParameterTuple<Function>::type m_parameters;
  };

  /**
   * Makes a FunctionEvaluatorNode.
   * @param function The function to evaluate.
   * @param args The parameters to pass to the <i>function</i>.
   */
  template<typename Function, typename... Args>
  std::unique_ptr<FunctionEvaluatorNode<Function>> MakeFunctionEvaluatorNode(
      const Function& function, std::unique_ptr<Args>... args) {
    return std::make_unique<FunctionEvaluatorNode<Function>>(function,
      std::move(args)...);
  }

  template<typename F>
  struct FunctionEvaluatorNodeTranslator {
    template<typename... Args>
    static BaseEvaluatorNode* Template(
        std::vector<std::unique_ptr<BaseEvaluatorNode>> parameters) {
      using Operation = typename F::template Operation<Args...>;
      return new FunctionEvaluatorNode<Operation>(Operation(),
        std::move(parameters));
    };

    using SupportedTypes = typename F::SupportedTypes;
  };

  template<typename F>
  FunctionEvaluatorNode<F>::Invoker::Invoker(const Function& function)
    : m_function(function) {}

  template<typename F>
  template<typename... Args>
  typename FunctionEvaluatorNode<F>::Result
      FunctionEvaluatorNode<F>::Invoker::operator ()(Args&... args) const {
    return m_function(args->Eval()...);
  }

  template<typename F>
  FunctionEvaluatorNode<F>::FunctionEvaluatorNode(const Function& function,
      std::vector<std::unique_ptr<BaseEvaluatorNode>> args)
      : m_invoker(function) {
    boost::fusion::for_each(m_parameters, Details::AssignParameters(args));
  }

  template<typename F>
  typename FunctionEvaluatorNode<F>::Result FunctionEvaluatorNode<F>::Eval() {
    return std::apply(m_invoker, m_parameters);
  }
}

#endif
