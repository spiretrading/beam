module;
#include "Prelude.hpp"

export module Beam:Evaluator;

export namespace Beam {
  class Evaluator;
}


export namespace Beam {

  /** Evaluates an and expression. */
  class AndEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      /**
       * Constructs an AndEvaluatorNode.
       * @param left The left hand side to evaluate.
       * @param right The right hand side to evaluate.
       */
      AndEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> left,
        std::unique_ptr<EvaluatorNode<bool>> right);

      bool eval() override;

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_left;
      std::unique_ptr<EvaluatorNode<bool>> m_right;
  };

  inline AndEvaluatorNode::AndEvaluatorNode(
    std::unique_ptr<EvaluatorNode<bool>> left,
    std::unique_ptr<EvaluatorNode<bool>> right)
    : m_left(std::move(left)),
      m_right(std::move(right)) {}

  inline bool AndEvaluatorNode::eval() {
    return m_left->eval() && m_right->eval();
  }
}


export namespace Beam {

  /**
   * Evaluates to a constant.
   * @tparam T The type of constant to return.
   */
  template<typename T>
  class ConstantEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a ConstantEvaluatorNode.
       * @param constant The constant to evaluate to.
       */
      explicit ConstantEvaluatorNode(Result constant);

      Result eval() override;

    private:
      Result m_constant;
  };

  /**
   * Translates a ConstantExpression into a ConstantEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ConstantEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    BaseEvaluatorNode* operator ()(const ConstantExpression& expression) const {
      return new ConstantEvaluatorNode(expression.get_value().as<T>());
    }
  };

  template<typename T>
  ConstantEvaluatorNode<T>::ConstantEvaluatorNode(Result constant)
    : m_constant(std::move(constant)) {}

  template<typename T>
  typename ConstantEvaluatorNode<T>::Result ConstantEvaluatorNode<T>::eval() {
    return m_constant;
  }
}


export namespace Beam {
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


export namespace Beam {

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


export namespace Beam {

  /** Evaluates a not expression. */
  class NotEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      /**
       * Constructs a NotEvaluatorNode.
       * @param operand The operand to evaluate.
       */
      explicit NotEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> operand);

      bool eval() override;

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_operand;
  };

  inline NotEvaluatorNode::NotEvaluatorNode(
    std::unique_ptr<EvaluatorNode<bool>> operand)
    : m_operand(std::move(operand)) {}

  inline bool NotEvaluatorNode::eval() {
    return !m_operand->eval();
  }
}


export namespace Beam {

  /** Evaluates an or expression. */
  class OrEvaluatorNode : public EvaluatorNode<bool> {
    public:
      using Result = bool;

      /**
       * Constructs an OrEvaluatorNode.
       * @param left The left hand side to evaluate.
       * @param right The right hand side to evaluate.
       */
      OrEvaluatorNode(std::unique_ptr<EvaluatorNode<bool>> left,
        std::unique_ptr<EvaluatorNode<bool>> right);

      bool eval() override;

    private:
      std::unique_ptr<EvaluatorNode<bool>> m_left;
      std::unique_ptr<EvaluatorNode<bool>> m_right;
  };

  inline OrEvaluatorNode::OrEvaluatorNode(
    std::unique_ptr<EvaluatorNode<bool>> left,
    std::unique_ptr<EvaluatorNode<bool>> right)
    : m_left(std::move(left)),
      m_right(std::move(right)) {}

  inline bool OrEvaluatorNode::eval() {
    return m_left->eval() || m_right->eval();
  }
}


export namespace Beam {

  /** Base class for the ParameterEvaluatorNode. */
  class BaseParameterEvaluatorNode {
    public:
      virtual ~BaseParameterEvaluatorNode() = default;

      /** Returns the type evaluated by this node. */
      virtual std::type_index get_type() const = 0;

      /** Returns the parameter index. */
      virtual int get_index() const = 0;

      /** Initializes the parameter to use when performing an evaluation. */
      virtual void set_parameter(const void** parameter) = 0;
  };

  /**
   * Evaluates to a supplied parameter.
   * @param T The type of parameter to return.
   */
  template<typename T>
  class ParameterEvaluatorNode :
      public EvaluatorNode<T>, public BaseParameterEvaluatorNode {
    public:
      using Result = T;

      /**
       * Constructs a ParameterEvaluatorNode with a given index.
       * @param index The parameter's index.
       */
      explicit ParameterEvaluatorNode(int index);

      std::type_index get_type() const override;
      int get_index() const override;
      void set_parameter(const void** parameter) override;
      Result eval() override;

    private:
      int m_index;
      const Result** m_parameter;
  };

  /**
   * Translates a ParameterExpression into a ParameterEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ParameterEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    BaseEvaluatorNode* operator ()(
        const ParameterExpression& expression) const {
      return new ParameterEvaluatorNode<T>(expression.get_index());
    }
  };

  template<typename T>
  ParameterEvaluatorNode<T>::ParameterEvaluatorNode(int index)
    : m_index(index),
      m_parameter(nullptr) {}

  template<typename T>
  std::type_index ParameterEvaluatorNode<T>::get_type() const {
    return EvaluatorNode<T>::get_type();
  }

  template<typename T>
  int ParameterEvaluatorNode<T>::get_index() const {
    return m_index;
  }

  template<typename T>
  void ParameterEvaluatorNode<T>::set_parameter(const void** parameter) {
    m_parameter = reinterpret_cast<const Result**>(parameter);
  }

  template<typename T>
  typename ParameterEvaluatorNode<T>::Result ParameterEvaluatorNode<T>::eval() {
    return **m_parameter;
  }
}


export namespace Beam {

  /**
   * Reads a value from a pointer.
   * @tparam T The type of data to read.
   */
  template<typename T>
  class ReadEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a ReadEvaluatorNode.
       * @param value The value to read.
       */
      explicit ReadEvaluatorNode(Result* value) noexcept;

      Result eval() override;

    private:
      Result* m_value;
  };

  /**
   * Translates a ReadExpression into a ReadEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ReadEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    std::unique_ptr<BaseEvaluatorNode> operator ()(void* address) const {
      return std::make_unique<ReadEvaluatorNode<T>>(static_cast<T*>(address));
    }
  };

  template<typename T>
  ReadEvaluatorNode<T>::ReadEvaluatorNode(Result* value) noexcept
    : m_value(value) {}

  template<typename T>
  typename ReadEvaluatorNode<T>::Result ReadEvaluatorNode<T>::eval() {
    return *m_value;
  }
}


export namespace Beam {
  class Evaluator;

  /**
   * Evaluates a ReduceExpression.
   * @tparam T The type being reduced.
   */
  template<typename T>
  class ReduceEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = typename EvaluatorNode<T>::Result;

      /**
       * Constructs a ReduceEvaluatorNode.
       * @param reducer The Evaluator used to perform the reduction.
       * @param series The series to reduce.
       * @param initial_value The initial value.
       */
      ReduceEvaluatorNode(std::unique_ptr<Evaluator> reducer,
        std::unique_ptr<EvaluatorNode<T>> series, Result initial_value);

      Result eval() override;

    private:
      std::unique_ptr<Evaluator> m_reducer;
      std::unique_ptr<EvaluatorNode<T>> m_series;
      Result m_value;
  };

  template<template<typename> class Node, typename T>
  ReduceEvaluatorNode(std::unique_ptr<Evaluator>, std::unique_ptr<Node<T>>,
    const typename EvaluatorNode<T>::Result&) -> ReduceEvaluatorNode<T>;

  /**
   * Translates a ReduceExpression into a ReduceEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ReduceEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    BaseEvaluatorNode* operator ()(std::unique_ptr<Evaluator> reducer,
        std::unique_ptr<BaseEvaluatorNode> series,
        const Value& initial_value) const {
      return new ReduceEvaluatorNode(
        std::move(reducer), static_pointer_cast<EvaluatorNode<T>>(
          std::move(series)), initial_value.as<T>());
    }
  };

  template<typename T>
  ReduceEvaluatorNode<T>::ReduceEvaluatorNode(
    std::unique_ptr<Evaluator> reducer,
    std::unique_ptr<EvaluatorNode<T>> series, Result initial_value)
    : m_reducer(std::move(reducer)),
      m_series(std::move(series)),
      m_value(std::move(initial_value)) {}
}


export namespace Beam {

  /**
   * Writes a value to a pointer.
   * @tparam T The type of data to write.
   */
  template<typename T>
  class WriteEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a WriteEvaluatorNode.
       * @param destination The destination to write to.
       * @param value The value to write.
       */
      WriteEvaluatorNode(
        Result* destination, std::unique_ptr<EvaluatorNode<Result>> value);

      Result eval() override;

    private:
      Result* m_destination;
      std::unique_ptr<EvaluatorNode<Result>> m_value;
  };

  /**
   * Translates a WriteExpression into a WriteEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct WriteEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    std::unique_ptr<BaseEvaluatorNode> operator ()(
        void* destination, std::unique_ptr<BaseEvaluatorNode> value) const {
      return std::make_unique<WriteEvaluatorNode<T>>(
        static_cast<T*>(destination),
        static_pointer_cast<EvaluatorNode<T>>(std::move(value)));
    }
  };

  template<typename R, typename Q>
  WriteEvaluatorNode(R*, std::unique_ptr<Q>) -> WriteEvaluatorNode<R>;

  template<typename T>
  WriteEvaluatorNode<T>::WriteEvaluatorNode(Result* destination,
    std::unique_ptr<EvaluatorNode<Result>> value)
    : m_destination(destination),
      m_value(std::move(value)) {}

  template<typename T>
  typename WriteEvaluatorNode<T>::Result WriteEvaluatorNode<T>::eval() {
    return *m_destination = m_value->eval();
  }
}


export namespace Beam {

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

export namespace Beam {

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

