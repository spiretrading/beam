#ifndef BEAM_EVALUATOR_TRANSLATOR_HPP
#define BEAM_EVALUATOR_TRANSLATOR_HPP
#include <array>
#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/mp11.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include <boost/variant/variant.hpp>
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
#include "Beam/Queries/ReadEvaluatorNode.hpp"
#include "Beam/Queries/ReduceEvaluatorNode.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Queries/WriteEvaluatorNode.hpp"
#include "Beam/Utilities/Casts.hpp"
#include "Beam/Utilities/Instantiate.hpp"

namespace Beam {

  /** The maximum number of supported parameters. */
  constexpr auto MAX_EVALUATOR_PARAMETERS = 2;

  /** A variant able to represent any query type. */
  using QueryVariant = boost::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;

  /** Wraps a QueryVariant into a SequencedValue. */
  using SequencedQueryVariant = SequencedValue<QueryVariant>;

  /** Stores typedefs of various types that can be used in an Expression. */
  struct QueryTypes {

    /** Lists all native types. */
    using NativeTypes = boost::mp11::mp_list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;

    /** Lists all value types. */
    using ValueTypes = boost::mp11::mp_list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;

    /** Lists types that can be compared. */
    using ComparableTypes = boost::mp11::mp_list<bool, char, int, double,
      std::uint64_t, std::string, boost::posix_time::ptime,
      boost::posix_time::time_duration>;
  };

  /**
   * Translates an Expression into an EvaluatorNode.
   * @tparam QueryTypes The list of types supported.
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
      void translate(const Expression& expression);

      /** Returns the EvaluatorNode that was last translated. */
      std::unique_ptr<BaseEvaluatorNode> get_evaluator();

      /** Returns the parameters that were translated. */
      const std::vector<BaseParameterEvaluatorNode*>& get_parameters() const;

      /**
       * Creates a new instance of this translator, typically used for
       * sub-expressions.
       */
      virtual std::unique_ptr<EvaluatorTranslator> make_translator() const;

    protected:

      /**
       * Sets the most recently translated evaluator.
       * @param evaluator The most recently translated evaluator.
       */
      void set_evaluator(std::unique_ptr<BaseEvaluatorNode> evaluator);

      void visit(const AndExpression& expression) override;
      void visit(const ConstantExpression& expression) override;
      void visit(const FunctionExpression& expression) override;
      void visit(
        const GlobalVariableDeclarationExpression& expression) override;
      void visit(const NotExpression& expression) override;
      void visit(const OrExpression& expression) override;
      void visit(const ParameterExpression& expression) override;
      void visit(const ReduceExpression& expression) override;
      void visit(const SetVariableExpression& expression) override;
      void visit(const VariableExpression& expression) override;
      void visit(const VirtualExpression& expression) override;

    private:
      struct VariableEntry {
        void* m_address;
        std::type_index m_type;

        VariableEntry(void* address, std::type_index type);
      };
      std::unique_ptr<BaseEvaluatorNode> m_evaluator;
      std::vector<BaseParameterEvaluatorNode*> m_parameters;
      std::unordered_map<std::string, std::vector<VariableEntry>> m_variables;

      const VariableEntry& find_variable(const std::string& name) const;
      template<typename Operation, int COUNT>
      void translate(const FunctionExpression& expression);
  };

  template<typename QueryTypes>
  EvaluatorTranslator<QueryTypes>::VariableEntry::VariableEntry(
    void* address, std::type_index type)
    : m_address(address),
      m_type(type) {}

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::translate(
      const Expression& expression) {
    expression.apply(*this);
    auto parameter_checks =
      std::array<boost::optional<std::type_index>, MAX_EVALUATOR_PARAMETERS>();
    auto max_index = -1;
    for(auto& parameter : m_parameters) {
      max_index = std::max(max_index, parameter->get_index());
      auto& check = parameter_checks[parameter->get_index()];
      if(check && check != parameter->get_type()) {
        boost::throw_with_location(
          ExpressionTranslationException("Parameter type mismatch."));
      } else {
        check = parameter->get_type();
      }
    }
    for(auto i = 0; i <= max_index; ++i) {
      if(!parameter_checks[i]) {
        boost::throw_with_location(
          ExpressionTranslationException("Missing parameter."));
      }
    }
  }

  template<typename QueryTypes>
  std::unique_ptr<BaseEvaluatorNode>
      EvaluatorTranslator<QueryTypes>::get_evaluator() {
    return std::move(m_evaluator);
  }

  template<typename QueryTypes>
  const std::vector<BaseParameterEvaluatorNode*>&
      EvaluatorTranslator<QueryTypes>::get_parameters() const {
    return m_parameters;
  }

  template<typename QueryTypes>
  std::unique_ptr<EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator<QueryTypes>::make_translator() const {
    return std::make_unique<EvaluatorTranslator>();
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::set_evaluator(
      std::unique_ptr<BaseEvaluatorNode> evaluator) {
    m_evaluator = std::move(evaluator);
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(const AndExpression& expression) {
    expression.get_left().apply(*this);
    auto left = static_pointer_cast<EvaluatorNode<bool>>(get_evaluator());
    expression.get_right().apply(*this);
    auto right = static_pointer_cast<EvaluatorNode<bool>>(get_evaluator());
    set_evaluator(
      std::make_unique<AndEvaluatorNode>(std::move(left), std::move(right)));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const ConstantExpression& expression) {
    m_evaluator.reset(instantiate<ConstantEvaluatorNodeTranslator<NativeTypes>>(
      expression.get_value().get_type())(expression));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const FunctionExpression& expression) {
    if(expression.get_name() == ADDITION_NAME) {
      translate<AdditionExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.get_name() == SUBTRACTION_NAME) {
      translate<SubtractionExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.get_name() == MULTIPLICATION_NAME) {
      translate<MultiplicationExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.get_name() == DIVISION_NAME) {
      translate<DivisionExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.get_name() == LESS_NAME) {
      translate<LessExpressionTranslator<ComparableTypes>, 2>(expression);
    } else if(expression.get_name() == LESS_EQUALS_NAME) {
      translate<LessEqualsExpressionTranslator<ComparableTypes>, 2>(expression);
    } else if(expression.get_name() == EQUALS_NAME) {
      translate<EqualsExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.get_name() == NOT_EQUALS_NAME) {
      translate<NotEqualsExpressionTranslator<NativeTypes>, 2>(expression);
    } else if(expression.get_name() == GREATER_EQUALS_NAME) {
      translate<GreaterEqualsExpressionTranslator<ComparableTypes>, 2>(
        expression);
    } else if(expression.get_name() == GREATER_NAME) {
      translate<GreaterExpressionTranslator<ComparableTypes>, 2>(expression);
    } else if(expression.get_name() == MAX_NAME) {
      translate<MaxExpressionTranslator<ComparableTypes>, 2>(expression);
    } else if(expression.get_name() == MIN_NAME) {
      translate<MinExpressionTranslator<ComparableTypes>, 2>(expression);
    } else {
      boost::throw_with_location(
        ExpressionTranslationException("Function not supported."));
    }
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const GlobalVariableDeclarationExpression& expression) {
    auto& initial_value_expression = expression.get_initial_value();
    initial_value_expression.apply(*this);
    auto initial_value_evaluator = get_evaluator();
    auto& body_expression = expression.get_body();
    auto address = static_cast<void*>(nullptr);
    auto global_variable_evaluator = instantiate<
      GlobalVariableDeclarationEvaluatorNodeTranslator<NativeTypes>>(
        initial_value_expression.get_type(), body_expression.get_type())(
          std::move(initial_value_evaluator), out(address));
    auto& variables = m_variables[expression.get_name()];
    variables.emplace_back(address, initial_value_expression.get_type());
    try {
      body_expression.apply(*this);
    } catch(const std::exception&) {
      variables.pop_back();
      throw;
    }
    auto body_evaluator = get_evaluator();
    variables.pop_back();
    instantiate<GlobalVariableDeclarationEvaluatorNodeSetBody<NativeTypes>>(
      initial_value_expression.get_type(), body_expression.get_type())(
        *global_variable_evaluator, std::move(body_evaluator));
    set_evaluator(std::move(global_variable_evaluator));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(const NotExpression& expression) {
    expression.get_operand().apply(*this);
    auto operand = static_pointer_cast<EvaluatorNode<bool>>(get_evaluator());
    set_evaluator(std::make_unique<NotEvaluatorNode>(std::move(operand)));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(const OrExpression& expression) {
    expression.get_left().apply(*this);
    auto left = static_pointer_cast<EvaluatorNode<bool>>(get_evaluator());
    expression.get_right().apply(*this);
    auto right = static_pointer_cast<EvaluatorNode<bool>>(get_evaluator());
    set_evaluator(
      std::make_unique<OrEvaluatorNode>(std::move(left), std::move(right)));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const ParameterExpression& expression) {
    if(expression.get_index() < 0 ||
        expression.get_index() >= MAX_EVALUATOR_PARAMETERS) {
      boost::throw_with_location(
        ExpressionTranslationException("Too many parameters."));
    }
    m_evaluator.reset(instantiate<
      ParameterEvaluatorNodeTranslator<NativeTypes>>(expression.get_type())(
        expression));
    m_parameters.push_back(dynamic_cast<BaseParameterEvaluatorNode*>(
      m_evaluator.get()));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const SetVariableExpression& expression) {
    auto& variable = find_variable(expression.get_name());
    if(variable.m_type != expression.get_type()) {
      boost::throw_with_location(
        ExpressionTranslationException("Type mismatch."));
    }
    expression.get_value().apply(*this);
    auto value_evaluator = get_evaluator();
    auto evaluator = instantiate<WriteEvaluatorNodeTranslator<NativeTypes>>(
      expression.get_type())(variable.m_address, std::move(value_evaluator));
    set_evaluator(std::move(evaluator));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const VariableExpression& expression) {
    auto& variable = find_variable(expression.get_name());
    if(variable.m_type != expression.get_type()) {
      boost::throw_with_location(
        ExpressionTranslationException("Type mismatch."));
    }
    auto evaluator = instantiate<ReadEvaluatorNodeTranslator<NativeTypes>>(
      expression.get_type())(variable.m_address);
    set_evaluator(std::move(evaluator));
  }

  template<typename QueryTypes>
  void EvaluatorTranslator<QueryTypes>::visit(
      const VirtualExpression& expression) {
    boost::throw_with_location(
      ExpressionTranslationException("Expression not supported."));
  }

  template<typename QueryTypes>
  const typename EvaluatorTranslator<QueryTypes>::VariableEntry&
      EvaluatorTranslator<QueryTypes>::find_variable(
        const std::string& name) const {
    auto i = m_variables.find(name);
    if(i == m_variables.end() || i->second.empty()) {
      boost::throw_with_location(
        ExpressionTranslationException("Variable not found."));
    }
    return i->second.back();
  }

  template<typename QueryTypes>
  template<typename Translator, int COUNT>
  void EvaluatorTranslator<QueryTypes>::translate(
      const FunctionExpression& expression) {
    if(expression.get_parameters().size() != COUNT) {
      boost::throw_with_location(
        ExpressionTranslationException("Invalid parameter count."));
    }
    auto parameters = std::vector<std::unique_ptr<BaseEvaluatorNode>>();
    for(auto& parameter : expression.get_parameters()) {
      parameter.apply(*this);
      parameters.push_back(std::move(m_evaluator));
    }
    try {
      if constexpr(COUNT == 1) {
        m_evaluator.reset(instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.get_parameters()[0].get_type())(std::move(parameters)));
      } else if constexpr(COUNT == 2) {
        m_evaluator.reset(instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.get_parameters()[0].get_type(),
            expression.get_parameters()[1].get_type())(std::move(parameters)));
      } else if constexpr(COUNT == 3) {
        m_evaluator.reset(instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.get_parameters()[0].get_type(),
            expression.get_parameters()[1].get_type(),
            expression.get_parameters()[2].get_type())(std::move(parameters)));
      } else if constexpr(COUNT == 4) {
        m_evaluator.reset(instantiate<
          FunctionEvaluatorNodeTranslator<Translator>>(
            expression.get_parameters()[0].get_type(),
            expression.get_parameters()[1].get_type(),
            expression.get_parameters()[2].get_type(),
            expression.get_parameters()[3].get_type())(std::move(parameters)));
      } else {
        std::throw_with_nested(
          ExpressionTranslationException("Type mismatch."));
      }
    } catch(const std::invalid_argument&) {
      std::throw_with_nested(ExpressionTranslationException("Type mismatch."));
    }
  }
}

#endif
