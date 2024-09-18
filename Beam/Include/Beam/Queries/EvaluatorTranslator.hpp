#ifndef BEAM_EVALUATOR_TRANSLATOR_HPP
#define BEAM_EVALUATOR_TRANSLATOR_HPP
#include <array>
#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/AndEvaluatorNode.hpp"
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/FunctionEvaluatorNode.hpp"
#include "Beam/Queries/GlobalVariableDeclarationEvaluatorNode.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/NotEvaluatorNode.hpp"
#include "Beam/Queries/OrEvaluatorNode.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/ParameterEvaluatorNode.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/ReadEvaluatorNode.hpp"
#include "Beam/Queries/ReduceEvaluatorNode.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Queries/WriteEvaluatorNode.hpp"
#include "Beam/Utilities/Casts.hpp"
#include "Beam/Utilities/InstantiateTemplate.hpp"

namespace Beam::Queries {

  /** The maximum number of supported parameters. */
  constexpr auto MAX_EVALUATOR_PARAMETERS = 2;

  /**
   * Translates an Expression into an EvaluatorNode.
   * @param <QueryTypes> The list of types supported.
   */
  template<typename QueryTypes>
  class EvaluatorTranslator : protected ExpressionVisitor {
    public:

      /** Lists all value types. */
      using ValueTypes = typename QueryTypes::ValueTypes;

      /** Lists all native types. */
      using NativeTypes = typename QueryTypes::NativeTypes;

      /** Lists types that can be compared. */
      using ComparableTypes = typename QueryTypes::ComparableTypes;

      /**
       * Translates an Expression.
       * @param expression The Expression to translate.
       */
      void Translate(const Expression& expression);

      /** Returns the EvaluatorNode that was last translated. */
      std::unique_ptr<BaseEvaluatorNode> GetEvaluator();

      /** Returns the parameters that were translated. */
      const std::vector<BaseParameterEvaluatorNode*>& GetParameters() const;

      /**
       * Creates a new instance of this translator, typically used for
       * sub-expressions.
       */
      virtual std::unique_ptr<EvaluatorTranslator> NewTranslator() const;

    protected:

      /**
       * Sets the most recently translated evaluator.
       * @param evaluator The most recently translated evaluator.
       */
      void SetEvaluator(std::unique_ptr<BaseEvaluatorNode> evaluator);

      void Visit(const AndExpression& expression) override;

      void Visit(const ConstantExpression& expression) override;

      void Visit(const FunctionExpression& expression) override;

      void Visit(
        const GlobalVariableDeclarationExpression& expression) override;

      void Visit(const NotExpression& expression) override;

      void Visit(const OrExpression& expression) override;

      void Visit(const ParameterExpression& expression) override;

      void Visit(const ReduceExpression& expression) override;

      void Visit(const SetVariableExpression& expression) override;

      void Visit(const VariableExpression& expression) override;

      void Visit(const VirtualExpression& expression) override;

    private:
      struct VariableEntry {
        void* m_address;
        const std::type_info* m_type;

        VariableEntry(void* address, const std::type_info& type);
      };
      std::unique_ptr<BaseEvaluatorNode> m_evaluator;
      std::vector<BaseParameterEvaluatorNode*> m_parameters;
      std::unordered_map<std::string, std::vector<VariableEntry>> m_variables;

      const VariableEntry& FindVariable(const std::string& name) const;
      template<typename Operation, int COUNT>
      void TranslateFunction(const FunctionExpression& expression);
  };

  template<typename QueryTypes>
  EvaluatorTranslator<QueryTypes>::VariableEntry::VariableEntry(void* address,
    const std::type_info& type)
    : m_address(address),
      m_type(&type) {}

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Translate(
      const Expression& expression) {
    expression->Apply(*this);
    auto parameterChecks = std::array<boost::optional<const std::type_info*>,
      MAX_EVALUATOR_PARAMETERS>();
    auto maxIndex = -1;
    for(auto& parameter : m_parameters) {
      maxIndex = std::max(maxIndex, parameter->GetIndex());
      auto& parameterCheck = parameterChecks[parameter->GetIndex()];
      if(parameterCheck) {
        if(**parameterCheck != parameter->GetResultType()) {
          BOOST_THROW_EXCEPTION(ExpressionTranslationException(
            "Parameter type mismatch."));
        }
      } else {
        parameterCheck = &parameter->GetResultType();
      }
    }
    for(auto i = 0; i <= maxIndex; ++i) {
      if(!parameterChecks[i]) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Missing parameter."));
      }
    }
  }

  template<typename QueryTypes>
  std::unique_ptr<BaseEvaluatorNode> EvaluatorTranslator<QueryTypes>::
      GetEvaluator() {
    return std::move(m_evaluator);
  }

  template<typename QueryTypes>
  const std::vector<BaseParameterEvaluatorNode*>&
      EvaluatorTranslator<QueryTypes>::GetParameters() const {
    return m_parameters;
  }

  template<typename QueryTypes>
  std::unique_ptr<EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator<QueryTypes>::NewTranslator() const {
    return std::make_unique<EvaluatorTranslator>();
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::SetEvaluator(
      std::unique_ptr<BaseEvaluatorNode> evaluator) {
    m_evaluator = std::move(evaluator);
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(const AndExpression& expression) {
    auto leftExpression = expression.GetLeftExpression();
    leftExpression->Apply(*this);
    auto leftEvaluator = Beam::StaticCast<std::unique_ptr<EvaluatorNode<bool>>>(
      GetEvaluator());
    auto rightExpression = expression.GetRightExpression();
    rightExpression->Apply(*this);
    auto rightEvaluator = Beam::StaticCast<
      std::unique_ptr<EvaluatorNode<bool>>>(GetEvaluator());
    SetEvaluator(std::make_unique<AndEvaluatorNode>(std::move(leftEvaluator),
      std::move(rightEvaluator)));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const ConstantExpression& expression) {
    m_evaluator.reset(Instantiate<ConstantEvaluatorNodeTranslator<NativeTypes>>(
      expression.GetType()->GetNativeType())(expression));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const FunctionExpression& expression) {
    if(expression.GetName() == ADDITION_NAME) {
      TranslateFunction<AdditionExpressionTranslator, 2>(expression);
    } else if(expression.GetName() == SUBTRACTION_NAME) {
      TranslateFunction<SubtractionExpressionTranslator, 2>(expression);
    } else if(expression.GetName() == MULTIPLICATION_NAME) {
      TranslateFunction<MultiplicationExpressionTranslator, 2>(expression);
    } else if(expression.GetName() == DIVISION_NAME) {
      TranslateFunction<DivisionExpressionTranslator, 2>(expression);
    } else if(expression.GetName() == LESS_NAME) {
      TranslateFunction<LessExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.GetName() == LESS_EQUALS_NAME) {
      TranslateFunction<LessEqualsExpressionTranslator<NativeTypes>, 2>(
        expression);
    } else if(expression.GetName() == EQUALS_NAME) {
      TranslateFunction<EqualsExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.GetName() == NOT_EQUALS_NAME) {
      TranslateFunction<NotEqualsExpressionTranslator<NativeTypes>, 2>(
        expression);
    } else if(expression.GetName() == GREATER_EQUALS_NAME) {
      TranslateFunction<GreaterEqualsExpressionTranslator<NativeTypes>, 2>(
        expression);
    } else if(expression.GetName() == GREATER_NAME) {
      TranslateFunction<GreaterExpressionTranslator<NativeTypes>, 2>(
        expression);
    } else if(expression.GetName() == MAX_NAME) {
      TranslateFunction<MaxExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.GetName() == MIN_NAME) {
      TranslateFunction<MinExpressionTranslator<NativeTypes>, 2>(expression);
    } else {
      BOOST_THROW_EXCEPTION(
        ExpressionTranslationException("Function not supported."));
    }
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const GlobalVariableDeclarationExpression& expression) {
    auto& initialValueExpression = expression.GetInitialValue();
    initialValueExpression->Apply(*this);
    auto initialValueEvaluator = std::move(GetEvaluator());
    auto& bodyExpression = expression.GetBody();
    auto address = static_cast<void*>(nullptr);
    auto globalVariableEvaluator = Instantiate<
      GlobalVariableDeclarationEvaluatorNodeTranslator<NativeTypes>>(
      initialValueExpression->GetType()->GetNativeType(),
      bodyExpression->GetType()->GetNativeType())(
      std::move(initialValueEvaluator), Store(address));
    auto& variables = m_variables[expression.GetName()];
    variables.emplace_back(address,
      initialValueExpression->GetType()->GetNativeType());
    try {
      bodyExpression->Apply(*this);
    } catch(const std::exception&) {
      variables.pop_back();
      throw;
    }
    auto bodyEvaluator = std::move(GetEvaluator());
    variables.pop_back();
    Instantiate<GlobalVariableDeclarationEvaluatorNodeSetBody<NativeTypes>>(
      initialValueExpression->GetType()->GetNativeType(),
      bodyExpression->GetType()->GetNativeType())(*globalVariableEvaluator,
      std::move(bodyEvaluator));
    SetEvaluator(std::move(globalVariableEvaluator));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(const NotExpression& expression) {
    auto operandExpression = expression.GetOperand();
    operandExpression->Apply(*this);
    auto operandEvaluator =
      Beam::StaticCast<std::unique_ptr<EvaluatorNode<bool>>>(GetEvaluator());
    SetEvaluator(
      std::make_unique<NotEvaluatorNode>(std::move(operandEvaluator)));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(const OrExpression& expression) {
    auto leftExpression = expression.GetLeftExpression();
    leftExpression->Apply(*this);
    auto leftEvaluator = Beam::StaticCast<std::unique_ptr<EvaluatorNode<bool>>>(
      GetEvaluator());
    auto rightExpression = expression.GetRightExpression();
    rightExpression->Apply(*this);
    auto rightEvaluator = Beam::StaticCast<
      std::unique_ptr<EvaluatorNode<bool>>>(GetEvaluator());
    SetEvaluator(std::make_unique<OrEvaluatorNode>(std::move(leftEvaluator),
      std::move(rightEvaluator)));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const ParameterExpression& expression) {
    if(expression.GetIndex() < 0 ||
        expression.GetIndex() >= MAX_EVALUATOR_PARAMETERS) {
      BOOST_THROW_EXCEPTION(
        ExpressionTranslationException("Too many parameters."));
    }
    m_evaluator.reset(Instantiate<
      ParameterEvaluatorNodeTranslator<NativeTypes>>(
      expression.GetType()->GetNativeType())(expression));
    m_parameters.push_back(dynamic_cast<BaseParameterEvaluatorNode*>(
      m_evaluator.get()));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const SetVariableExpression& expression) {
    auto& variable = FindVariable(expression.GetName());
    if(*variable.m_type != expression.GetType()->GetNativeType()) {
      BOOST_THROW_EXCEPTION(ExpressionTranslationException("Type mismatch."));
    }
    auto valueExpression = expression.GetValue();
    valueExpression->Apply(*this);
    auto valueEvaluator = std::move(GetEvaluator());
    auto evaluator = Instantiate<WriteEvaluatorNodeTranslator<NativeTypes>>(
      expression.GetType()->GetNativeType())(variable.m_address,
      std::move(valueEvaluator));
    SetEvaluator(std::move(evaluator));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const VariableExpression& expression) {
    auto& variable = FindVariable(expression.GetName());
    if(*variable.m_type != expression.GetType()->GetNativeType()) {
      BOOST_THROW_EXCEPTION(ExpressionTranslationException("Type mismatch."));
    }
    auto evaluator = Instantiate<ReadEvaluatorNodeTranslator<NativeTypes>>(
      expression.GetType()->GetNativeType())(variable.m_address);
    SetEvaluator(std::move(evaluator));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const VirtualExpression& expression) {
    BOOST_THROW_EXCEPTION(ExpressionTranslationException(
      "Expression not supported."));
  }

  template<typename QueryTypes>
  const typename EvaluatorTranslator<QueryTypes>::VariableEntry&
      EvaluatorTranslator<QueryTypes>::FindVariable(
      const std::string& name) const {
    auto variableIterator = m_variables.find(name);
    if(variableIterator == m_variables.end() ||
        variableIterator->second.empty()) {
      BOOST_THROW_EXCEPTION(ExpressionTranslationException(
        "Variable not found."));
    }
    return variableIterator->second.back();
  }

  template<typename QueryTypes>
  template<typename Translator, int COUNT>
  void EvaluatorTranslator<QueryTypes>::TranslateFunction(
      const FunctionExpression& expression) {
    if(expression.GetParameters().size() != COUNT) {
      BOOST_THROW_EXCEPTION(
        ExpressionTranslationException("Invalid parameter count."));
    }
    auto parameters = std::vector<std::unique_ptr<BaseEvaluatorNode>>();
    for(auto& parameter : expression.GetParameters()) {
      parameter->Apply(*this);
      parameters.push_back(std::move(m_evaluator));
    }
    try {
      if constexpr(COUNT == 1) {
        m_evaluator.reset(Instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.GetParameters()[0]->GetType()->GetNativeType())(
              std::move(parameters)));
      } else if constexpr(COUNT == 2) {
        m_evaluator.reset(Instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.GetParameters()[0]->GetType()->GetNativeType(),
            expression.GetParameters()[1]->GetType()->GetNativeType())(
              std::move(parameters)));
      } else if constexpr(COUNT == 3) {
        m_evaluator.reset(Instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.GetParameters()[0]->GetType()->GetNativeType(),
            expression.GetParameters()[1]->GetType()->GetNativeType(),
            expression.GetParameters()[2]->GetType()->GetNativeType())(
              std::move(parameters)));
      } else if constexpr(COUNT == 4) {
        m_evaluator.reset(Instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.GetParameters()[0]->GetType()->GetNativeType(),
            expression.GetParameters()[1]->GetType()->GetNativeType(),
            expression.GetParameters()[2]->GetType()->GetNativeType(),
            expression.GetParameters()[3]->GetType()->GetNativeType())(
              std::move(parameters)));
      } else {
        std::throw_with_nested(
          ExpressionTranslationException("Type mismatch."));
      }
    } catch(const InstantiationNotSupportedException&) {
      std::throw_with_nested(ExpressionTranslationException("Type mismatch."));
    }
  }
}

#endif
