#ifndef BEAM_QUERYEVALUATOR_HPP
#define BEAM_QUERYEVALUATOR_HPP
#include <array>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ParameterEvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class Evaluator
      \brief Evaluates an Expression.
   */
  class Evaluator : private boost::noncopyable {
    public:

      //! Constructs an Evaluator.
      /*!
        \param evaluator The EvaluatorNode at the root of the evaluation.
        \param parameters The parameters used in the evaluation.
      */
      Evaluator(std::unique_ptr<BaseEvaluatorNode> evaluator,
        const std::vector<BaseParameterEvaluatorNode*>& parameters);

      //! Evaluates the Expression.
      /*!
        \return The result of the evaluation.
      */
      template<typename Result>
      Result Eval();

      //! Evaluates the Expression.
      /*!
        \param parameter The parameter to apply.
        \return The result of the evaluation.
      */
      template<typename Result, typename Parameter>
      Result Eval(const Parameter& parameter);

      //! Evaluates the Expression.
      /*!
        \param p1 The first parameter to apply.
        \param p2 The second parameter to apply.
        \return The result of the evaluation.
      */
      template<typename Result, typename P1, typename P2>
      Result Eval(const P1& p1, const P2& p2);

    private:
      std::unique_ptr<BaseEvaluatorNode> m_evaluator;
      std::array<const void*, MAX_EVALUATOR_PARAMETERS> m_parameters;
  };

  //! Translates an Expression into an Evaluator.
  /*!
    \param expression The Expression to translate.
    \param translator The EvaluatorTranslator to use.
    \return An Evaluator representing the translated <i>expression</i>.
  */
  template<typename Translator>
  std::unique_ptr<Evaluator> Translate(const Expression& expression,
      Translator& translator) {
    translator.Translate(expression);
    auto evaluator = std::make_unique<Evaluator>(
      std::move(translator.GetEvaluator()), translator.GetParameters());
    return evaluator;
  }

  //! Translates an Expression into an Evaluator.
  /*!
    \param expression The Expression to translate.
    \return An Evaluator representing the translated <i>expression</i>.
  */
  template<typename Translator = EvaluatorTranslator<QueryTypes>,
    typename... Args>
  std::unique_ptr<Evaluator> Translate(const Expression& expression,
      Args&&... args) {
    Translator translator{std::forward<Args>(args)...};
    return Translate(expression, translator);
  }

  inline Evaluator::Evaluator(std::unique_ptr<BaseEvaluatorNode> evaluator,
      const std::vector<BaseParameterEvaluatorNode*>& parameters)
      : m_evaluator(std::move(evaluator)) {
    m_parameters.fill(nullptr);
    for(auto& node : parameters) {
      node->SetParameter(&m_parameters[node->GetIndex()]);
    }
  }

  template<typename Result>
  Result Evaluator::Eval() {
    return static_cast<EvaluatorNode<Result>*>(m_evaluator.get())->Eval();
  }

  template<typename Result, typename Parameter>
  Result Evaluator::Eval(const Parameter& parameter) {
    m_parameters[0] = &parameter;
    return this->Eval<Result>();
  }

  template<typename Result, typename P1, typename P2>
  Result Evaluator::Eval(const P1& p1, const P2& p2) {
    m_parameters[0] = &p1;
    m_parameters[1] = &p2;
    return this->Eval<Result>();
  }

  template<typename TypeList>
  struct ReduceEvaluatorNodeTranslator {
    template<typename T>
    static BaseEvaluatorNode* Template(std::unique_ptr<Evaluator> reducer,
        std::unique_ptr<BaseEvaluatorNode> series, const Value& initialValue) {
      return new ReduceEvaluatorNode<T>(std::move(reducer),
        StaticCast<std::unique_ptr<EvaluatorNode<T>>>(std::move(series)),
        initialValue->GetValue<T>());
    }

    using SupportedTypes = TypeList;
  };

  template<typename T>
  typename ReduceEvaluatorNode<T>::Result ReduceEvaluatorNode<T>::Eval() {
    m_value = m_reducer->template Eval<Result>(m_value, m_series->Eval());
    return m_value;
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const ReduceExpression& expression) {
    std::unique_ptr<EvaluatorTranslator<QueryTypes>> translator =
      NewTranslator();
    std::unique_ptr<Evaluator> evaluator = Queries::Translate(
      expression.GetReduceExpression(), *translator);
    expression.GetSeriesExpression()->Apply(*this);
    std::unique_ptr<BaseEvaluatorNode> series = std::move(m_evaluator);
    m_evaluator.reset(Instantiate<ReduceEvaluatorNodeTranslator<NativeTypes>>(
      expression.GetReduceExpression()->GetType()->GetNativeType())(
      std::move(evaluator), std::move(series), expression.GetInitialValue()));
  }
}
}

#endif
