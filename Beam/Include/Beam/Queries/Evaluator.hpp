#ifndef BEAM_QUERY_EVALUATOR_HPP
#define BEAM_QUERY_EVALUATOR_HPP
#include <array>
#include <memory>
#include <typeindex>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"

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
      struct ParameterEntry {
        const void* m_value = nullptr;
        boost::optional<std::type_index> m_type;
      };
      std::unique_ptr<BaseEvaluatorNode> m_evaluator;
      std::array<ParameterEntry, MAX_EVALUATOR_PARAMETERS> m_parameters;

      Evaluator(const Evaluator&) = delete;
      Evaluator& operator =(const Evaluator&) = delete;
      void check_parameter(int index, std::type_index type) const;
  };

  /**
   * Translates an Expression into an Evaluator.
   * @param expression The Expression to translate.
   * @param translator The EvaluatorTranslator to use.
   * @return An Evaluator representing the translated <i>expression</i>.
   */
  template<typename Translator> requires requires(Translator& translator) {
    translator.take_evaluator();
  }
  std::unique_ptr<Evaluator> translate(
      const Expression& expression, Translator& translator) {
    translator.translate(expression);
    return std::make_unique<Evaluator>(
      translator.take_evaluator(), translator.get_parameters());
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
    for(auto& node : parameters) {
      auto& entry = m_parameters[node->get_index()];
      node->set_parameter(&entry.m_value);
      entry.m_type = node->get_type();
    }
  }

  inline void Evaluator::check_parameter(
      int index, std::type_index type) const {
    auto& entry = m_parameters[index];
    if(entry.m_type && *entry.m_type != type) {
      boost::throw_with_location(
        TypeCompatibilityException("Parameter type mismatch."));
    }
  }

  template<typename Result>
  Result Evaluator::eval() {
    return static_cast<EvaluatorNode<Result>*>(m_evaluator.get())->eval();
  }

  template<typename Result, typename Parameter>
  Result Evaluator::eval(const Parameter& parameter) {
    check_parameter(0, typeid(Parameter));
    m_parameters[0].m_value = &parameter;
    return this->eval<Result>();
  }

  template<typename Result, typename P1, typename P2>
  Result Evaluator::eval(const P1& p1, const P2& p2) {
    check_parameter(0, typeid(P1));
    check_parameter(1, typeid(P2));
    m_parameters[0].m_value = &p1;
    m_parameters[1].m_value = &p2;
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
    auto translator = make_translator(expression.get_reducer().get_type());
    auto evaluator = Beam::translate(expression.get_reducer(), *translator);
    auto series = translate_operand(expression.get_series());
    m_evaluator.reset(instantiate<ReduceEvaluatorNodeTranslator<NativeTypes>>(
      expression.get_reducer().get_type())(std::move(evaluator),
        std::move(series), expression.get_initial_value()));
  }
}

#endif
