#ifndef BEAM_QUERY_EVALUATOR_HPP
#define BEAM_QUERY_EVALUATOR_HPP
#include <array>
#include <memory>
#include <vector>
#include "Beam/Queries/EvaluatorTranslator.hpp"

namespace Beam {

  /** Evaluates an Expression. */
  class Evaluator {
    public:

      /**
       * Constructs an Evaluator.
       * @param evaluator The EvaluatorNode at the root of the evaluation.
       * @param parameters The parameters used in the evaluation.
       */
      Evaluator(std::unique_ptr<BaseEvaluatorNode> evaluator,
        const std::vector<BaseParameterEvaluatorNode*>& parameters);

      /**
       * Evaluates the Expression.
       * @return The result of the evaluation.
       */
      template<typename Result>
      Result eval();

      /**
       * Evaluates the Expression.
       * @param parameter The parameter to apply.
       * @return The result of the evaluation.
       */
      template<typename Result, typename Parameter>
      Result eval(const Parameter& parameter);

      /**
       * Evaluates the Expression.
       * @param p1 The first parameter to apply.
       * @param p2 The second parameter to apply.
       * @return The result of the evaluation.
       */
      template<typename Result, typename P1, typename P2>
      Result eval(const P1& p1, const P2& p2);

    private:
      std::unique_ptr<BaseEvaluatorNode> m_evaluator;
      std::array<const void*, MAX_EVALUATOR_PARAMETERS> m_parameters;

      Evaluator(const Evaluator&) = delete;
      Evaluator& operator =(const Evaluator&) = delete;
  };

  /**
   * Translates an Expression into an Evaluator.
   * @param expression The Expression to translate.
   * @param translator The EvaluatorTranslator to use.
   * @return An Evaluator representing the translated <i>expression</i>.
   */
  template<typename Translator>
  std::unique_ptr<Evaluator> translate(
      const Expression& expression, Translator& translator) {
    translator.translate(expression);
    return std::make_unique<Evaluator>(
      translator.get_evaluator(), translator.get_parameters());
  }

  /**
   * Translates an Expression into an Evaluator.
   * @param expression The Expression to translate.
   * @return An Evaluator representing the translated <i>expression</i>.
   */
  template<typename Translator = EvaluatorTranslator<QueryTypes>,
    typename... Args>
  std::unique_ptr<Evaluator> translate(const Expression& expression,
      Args&&... args) {
    auto translator = Translator(std::forward<Args>(args)...);
    return translate(expression, translator);
  }

  inline Evaluator::Evaluator(std::unique_ptr<BaseEvaluatorNode> evaluator,
      const std::vector<BaseParameterEvaluatorNode*>& parameters)
      : m_evaluator(std::move(evaluator)) {
    m_parameters.fill(nullptr);
    for(auto& node : parameters) {
      node->set_parameter(&m_parameters[node->get_index()]);
    }
  }

  template<typename Result>
  Result Evaluator::eval() {
    return static_cast<EvaluatorNode<Result>*>(m_evaluator.get())->eval();
  }

  template<typename Result, typename Parameter>
  Result Evaluator::eval(const Parameter& parameter) {
    m_parameters[0] = &parameter;
    return this->eval<Result>();
  }

  template<typename Result, typename P1, typename P2>
  Result Evaluator::eval(const P1& p1, const P2& p2) {
    m_parameters[0] = &p1;
    m_parameters[1] = &p2;
    return this->eval<Result>();
  }

  template<typename T>
  typename ReduceEvaluatorNode<T>::Result ReduceEvaluatorNode<T>::eval() {
    m_value = m_reducer->template eval<Result>(m_value, m_series->eval());
    return m_value;
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const ReduceExpression& expression) {
    auto translator = make_translator();
    auto evaluator = Beam::translate(expression.get_reducer(), *translator);
    expression.get_series().apply(*this);
    m_evaluator.reset(instantiate<ReduceEvaluatorNodeTranslator<NativeTypes>>(
      expression.get_reducer().get_type())(std::move(evaluator),
        std::move(std::move(m_evaluator)), expression.get_initial_value()));
  }
}

#endif
