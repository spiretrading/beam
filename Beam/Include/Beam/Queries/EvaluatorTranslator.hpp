#ifndef BEAM_EVALUATORTRANSLATOR_HPP
#define BEAM_EVALUATORTRANSLATOR_HPP
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/FunctionEvaluatorNode.hpp"
#include "Beam/Queries/GlobalVariableDeclarationEvaluatorNode.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
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

namespace Beam {
namespace Queries {

  //! The maximum number of supported parameters.
  const int MAX_EVALUATOR_PARAMETERS = 2;

  /*! \class EvaluatorTranslator
      \brief Translates an Expression into an EvaluatorNode.
      \tparam QueryTypes The list of types supported.
   */
  template<typename QueryTypes>
  class EvaluatorTranslator : public ExpressionVisitor {
    public:

      //! Lists all value types.
      using ValueTypes = typename QueryTypes::ValueTypes;

      //! Lists all native types.
      using NativeTypes = typename QueryTypes::NativeTypes;

      //! Lists types that can be compared.
      using ComparableTypes = typename QueryTypes::ComparableTypes;

      //! Translates an Expression.
      /*!
        \param expression The Expression to translate.
      */
      void Translate(const Expression& expression);

      //! Returns the EvaluatorNode that was last translated.
      std::unique_ptr<BaseEvaluatorNode> GetEvaluator();

      //! Returns the parameters that were translated.
      const std::vector<BaseParameterEvaluatorNode*>& GetParameters() const;

      //! Creates a new instance of this translator, typically used for
      //! sub-expressions.
      virtual std::unique_ptr<EvaluatorTranslator> NewTranslator() const;

      virtual void Visit(const ConstantExpression& expression);

      virtual void Visit(const FunctionExpression& expression);

      virtual void Visit(const GlobalVariableDeclarationExpression& expression);

      virtual void Visit(const OrExpression& expression);

      virtual void Visit(const ParameterExpression& expression);

      virtual void Visit(const ReduceExpression& expression);

      virtual void Visit(const SetVariableExpression& expression);

      virtual void Visit(const VariableExpression& expression);

      virtual void Visit(const VirtualExpression& expression);

    protected:

      //! Sets the most recently translated evaluator.
      /*!
        \param evaluator The most recently translated evaluator.
      */
      void SetEvaluator(std::unique_ptr<BaseEvaluatorNode> evaluator);

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
    std::array<boost::optional<const std::type_info*>, 2> parameterChecks;
    auto maxIndex = -1;
    for(const auto& parameter : m_parameters) {
      maxIndex = std::max(maxIndex, parameter->GetIndex());
      auto& parameterCheck = parameterChecks[parameter->GetIndex()];
      if(parameterCheck.is_initialized()) {
        if(**parameterCheck != parameter->GetResultType()) {
          BOOST_THROW_EXCEPTION(ExpressionTranslationException(
            "Parameter type mismatch."));
        }
      } else {
        parameterCheck = &parameter->GetResultType();
      }
    }
    for(auto i = 0; i <= maxIndex; ++i) {
      if(!parameterChecks[i].is_initialized()) {
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
    return std::make_unique<EvaluatorTranslator<QueryTypes>>();
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
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      const auto& leftExpression = expression.GetParameters()[0];
      leftExpression->Apply(*this);
      std::vector<std::unique_ptr<BaseEvaluatorNode>> parameters;
      parameters.push_back(std::move(m_evaluator));
      const auto& rightExpression = expression.GetParameters()[1];
      rightExpression->Apply(*this);
      parameters.push_back(std::move(m_evaluator));
      m_evaluator.reset(Instantiate<
        FunctionEvaluatorNodeTranslator<AdditionExpressionTranslator>>(
        leftExpression->GetType()->GetNativeType(),
        rightExpression->GetType()->GetNativeType())(std::move(parameters)));
    } else if(expression.GetName() == EQUALS_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      const auto& leftExpression = expression.GetParameters()[0];
      leftExpression->Apply(*this);
      std::vector<std::unique_ptr<BaseEvaluatorNode>> parameters;
      parameters.push_back(std::move(m_evaluator));
      const auto& rightExpression = expression.GetParameters()[1];
      rightExpression->Apply(*this);
      parameters.push_back(std::move(m_evaluator));
      m_evaluator.reset(Instantiate<FunctionEvaluatorNodeTranslator<
        EqualsExpressionTranslator<NativeTypes>>>(
        leftExpression->GetType()->GetNativeType(),
        rightExpression->GetType()->GetNativeType())(std::move(parameters)));
    } else if(expression.GetName() == MAX_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      const auto& leftExpression = expression.GetParameters()[0];
      leftExpression->Apply(*this);
      std::vector<std::unique_ptr<BaseEvaluatorNode>> parameters;
      parameters.push_back(std::move(m_evaluator));
      const auto& rightExpression = expression.GetParameters()[1];
      rightExpression->Apply(*this);
      parameters.push_back(std::move(m_evaluator));
      m_evaluator.reset(Instantiate<FunctionEvaluatorNodeTranslator<
        MaxExpressionTranslator<NativeTypes>>>(
        leftExpression->GetType()->GetNativeType(),
        rightExpression->GetType()->GetNativeType())(std::move(parameters)));
    } else if(expression.GetName() == MIN_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      const auto& leftExpression = expression.GetParameters()[0];
      leftExpression->Apply(*this);
      std::vector<std::unique_ptr<BaseEvaluatorNode>> parameters;
      parameters.push_back(std::move(m_evaluator));
      const auto& rightExpression = expression.GetParameters()[1];
      rightExpression->Apply(*this);
      parameters.push_back(std::move(m_evaluator));
      m_evaluator.reset(Instantiate<FunctionEvaluatorNodeTranslator<
        MinExpressionTranslator<NativeTypes>>>(
        leftExpression->GetType()->GetNativeType(),
        rightExpression->GetType()->GetNativeType())(std::move(parameters)));
    } else {
      BOOST_THROW_EXCEPTION(ExpressionTranslationException(
        "Function not supported."));
    }
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::Visit(
      const GlobalVariableDeclarationExpression& expression) {
    const auto& initialValueExpression = expression.GetInitialValue();
    initialValueExpression->Apply(*this);
    auto initialValueEvaluator = std::move(GetEvaluator());
    const auto& bodyExpression = expression.GetBody();
    void* address;
    auto globalVariableEvaluator(Instantiate<
      GlobalVariableDeclarationEvaluatorNodeTranslator<NativeTypes>>(
      initialValueExpression->GetType()->GetNativeType(),
      bodyExpression->GetType()->GetNativeType())(
      std::move(initialValueEvaluator), Store(address)));
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
    const auto& variable = FindVariable(expression.GetName());
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
    const auto& variable = FindVariable(expression.GetName());
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
  void EvaluatorTranslator<QueryTypes>::SetEvaluator(
      std::unique_ptr<BaseEvaluatorNode> evaluator) {
    m_evaluator = std::move(evaluator);
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
}
}

#endif
