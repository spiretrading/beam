#ifndef BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#define BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#include <string>
#include <type_traits>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/mp11.hpp>
#include "Beam/Queries/FunctionExpression.hpp"

namespace Beam {
namespace Details {
  template<class T>
  struct has_less : std::bool_constant<requires (const T& a) {
    { a < a } -> std::convertible_to<bool>;
  }> {};

  template<typename TypeList, std::size_t I, std::size_t J, std::size_t Size,
    template<typename, typename> class HasOperation>
  struct generate_pairs_impl {
    using current_type1 = boost::mp11::mp_at_c<TypeList, I>;
    using current_type2 = boost::mp11::mp_at_c<TypeList, J>;
    using next_pairs = typename std::conditional_t<J + 1 < Size,
      generate_pairs_impl<TypeList, I, J + 1, Size, HasOperation>,
      std::conditional_t<I + 1 < Size,
        generate_pairs_impl<TypeList, I + 1, I + 1, Size, HasOperation>,
        std::type_identity<boost::mp11::mp_list<>>>>::type;
    using type = std::conditional_t<
      HasOperation<current_type1, current_type2>::value,
      boost::mp11::mp_push_back<
        next_pairs, boost::mp11::mp_list<current_type1, current_type2>>,
      next_pairs>;
  };

  template<typename TypeList, std::size_t Size,
    template<typename, typename> class HasOperation>
  struct generate_pairs_impl<TypeList, Size, Size, Size, HasOperation> {
    using type = boost::mp11::mp_list<>;
  };

  template<typename TypeList, template<typename, typename> class HasOperation>
  struct generate_pairs {
    static constexpr auto size = boost::mp11::mp_size<TypeList>::value;
    using type = std::conditional_t<size == 0,
      boost::mp11::mp_list<>,
      typename generate_pairs_impl<TypeList, 0, 0, size, HasOperation>::type>;
  };

  template<typename T, template<typename, typename> class HasOperation>
  struct make_parameter_list {
    using type = typename generate_pairs<T, HasOperation>::type;
  };
}

  /** The name used for the addition function. */
  inline const auto ADDITION_NAME = std::string("+");

  /** The name used for the subtraction function. */
  inline const auto SUBTRACTION_NAME = std::string("-");

  /** The name used for the multiplication function. */
  inline const auto MULTIPLICATION_NAME = std::string("*");

  /** The name used for the division function. */
  inline const auto DIVISION_NAME = std::string("/");

  /** The name used for the less function. */
  inline const auto LESS_NAME = std::string("<");

  /** The name used for the less or equals function. */
  inline const auto LESS_EQUALS_NAME = std::string("<=");

  /** The name used for the equals function. */
  inline const auto EQUALS_NAME = std::string("==");

  /** The name used for the inequality function. */
  inline const auto NOT_EQUALS_NAME = std::string("!=");

  /** The name used for the greater or equals function. */
  inline const auto GREATER_EQUALS_NAME = std::string(">=");

  /** The name used for the greater function. */
  inline const auto GREATER_NAME = std::string(">");

  /** The name used for the max function. */
  inline const auto MAX_NAME = std::string("max");

  /** The name used for the min function. */
  inline const auto MIN_NAME = std::string("min");

  /**
   * Constructs a FunctionExpression representing addition.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing addition.
   */
  inline FunctionExpression operator +(
      const Expression& left, const Expression& right) {
    auto type = [&] {
      if(left.get_type() == typeid(int) && right.get_type() == typeid(double)) {
        return std::type_index(typeid(double));
      }
      return left.get_type();
    }();
    return FunctionExpression(ADDITION_NAME, type, {left, right});
  }

  /**
   * Translates addition expressions.
   * @tparam ValueTypes The list of types that support addition.
   */
  template<typename ValueTypes>
  struct AdditionExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      { std::declval<A1>() + std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left + right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing subtraction.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing subtraction.
   */
  inline FunctionExpression operator -(
      const Expression& left, const Expression& right) {
    auto type = [&] {
      if(left.get_type() == typeid(int) && right.get_type() == typeid(double)) {
        return std::type_index(typeid(double));
      }
      return left.get_type();
    }();
    return FunctionExpression(SUBTRACTION_NAME, type, {left, right});
  }

  /**
   * Translates subtraction expressions.
   * @tparam ValueTypes The list of types that support subtraction.
   */
  template<typename ValueTypes>
  struct SubtractionExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      { std::declval<A1>() - std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left - right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing multiplication.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing multiplication.
   */
  inline FunctionExpression operator *(
      const Expression& left, const Expression& right) {
    auto type = [&] {
      if(left.get_type() == typeid(int) && right.get_type() == typeid(double)) {
        return std::type_index(typeid(double));
      }
      return left.get_type();
    }();
    return FunctionExpression(MULTIPLICATION_NAME, type, {left, right});
  }

  /**
   * Translates multiplication expressions.
   * @tparam ValueTypes The list of types that support multiplication.
   */
  template<typename ValueTypes>
  struct MultiplicationExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires !std::is_same_v<A1, boost::posix_time::time_duration>;
      requires !std::is_same_v<A2, boost::posix_time::time_duration>;
      { std::declval<A1>() * std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left * right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing division.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing division.
   */
  inline FunctionExpression operator /(
      const Expression& left, const Expression& right) {
    auto type = [&] {
      if(left.get_type() == typeid(int) && right.get_type() == typeid(double)) {
        return std::type_index(typeid(double));
      }
      return left.get_type();
    }();
    return FunctionExpression(DIVISION_NAME, type, {left, right});
  }

  /**
   * Translates division expressions.
   * @tparam ValueTypes The list of types that support division.
   */
  template<typename ValueTypes>
  struct DivisionExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires !std::is_same_v<A1, bool>;
      requires !std::is_same_v<A2, bool>;
      requires !std::is_same_v<A1, boost::posix_time::time_duration>;
      requires !std::is_same_v<A2, boost::posix_time::time_duration>;
      { std::declval<A1>() / std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left / right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing less.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing less.
   */
  inline FunctionExpression operator <(
      const Expression& left, const Expression& right) {
    return FunctionExpression(LESS_NAME, typeid(bool), {left, right});
  }

  /**
   * Translates less than expressions.
   * @tparam ValueTypes The list of types that support less than.
   */
  template<typename ValueTypes>
  struct LessExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires !std::is_same_v<A1, bool>;
      requires !std::is_same_v<A2, bool>;
      { std::declval<A1>() < std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left < right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing less than or equals.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing less than or equals.
   */
  inline FunctionExpression operator <=(
      const Expression& left, const Expression& right) {
    return FunctionExpression(LESS_EQUALS_NAME, typeid(bool), {left, right});
  }

  /**
   * Translates less than or equal expressions.
   * @tparam ValueTypes The list of types that support less than or equal.
   */
  template<typename ValueTypes>
  struct LessEqualsExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires !std::is_same_v<A1, bool>;
      requires !std::is_same_v<A2, bool>;
      { std::declval<A1>() <= std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left <= right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing equality.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing equality.
   */
  inline FunctionExpression operator ==(
      const Expression& left, const Expression& right) {
    return FunctionExpression(EQUALS_NAME, typeid(bool), {left, right});
  }

  /**
   * Translates equals expressions.
   * @tparam ValueTypes The list of types that support equals.
   */
  template<typename ValueTypes>
  struct EqualsExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires std::is_same_v<A1, A2>;
      { std::declval<A1>() == std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left == right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing inequality.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing inequality.
   */
  inline FunctionExpression operator !=(
      const Expression& left, const Expression& right) {
    return FunctionExpression(NOT_EQUALS_NAME, typeid(bool), {left, right});
  }

  /**
   * Translates not equals expressions.
   * @tparam ValueTypes The list of types that support not equals.
   */
  template<typename ValueTypes>
  struct NotEqualsExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires std::is_same_v<A1, A2>;
      { std::declval<A1>() != std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left != right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing greater or equals.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing greater or equals.
   */
  inline FunctionExpression operator >=(
      const Expression& left, const Expression& right) {
    return FunctionExpression(GREATER_EQUALS_NAME, typeid(bool), {left, right});
  }

  /**
   * Translates greater than or equals expressions.
   * @tparam ValueTypes The list of types that support greater than or equals.
   */
  template<typename ValueTypes>
  struct GreaterEqualsExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires !std::is_same_v<A1, bool>;
      requires !std::is_same_v<A2, bool>;
      { std::declval<A1>() >= std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left >= right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing greater.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing greater.
   */
  inline FunctionExpression operator >(
      const Expression& left, const Expression& right) {
    return FunctionExpression(GREATER_NAME, typeid(bool), {left, right});
  }

  /**
   * Translates greater than expressions.
   * @tparam ValueTypes The list of types that support greater than.
   */
  template<typename ValueTypes>
  struct GreaterExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires !std::is_same_v<A1, bool>;
      requires !std::is_same_v<A2, bool>;
      { (std::declval<A1>() > std::declval<A2>()) };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        return left > right;
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing the max function.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing the max function.
   */
  inline FunctionExpression max(
      const Expression& left, const Expression& right) {
    return FunctionExpression(MAX_NAME, left.get_type(), {left, right});
  }

  /**
   * Translates max expressions.
   * @tparam ValueTypes The list of types that support max.
   */
  template<typename ValueTypes>
  struct MaxExpressionTranslator {
    template<typename T>
    using make_signature = boost::mp11::mp_list<T, T>;

    using type = boost::mp11::mp_transform<make_signature,
      boost::mp11::mp_copy_if<ValueTypes, Details::has_less>>;

    template<typename T0, typename T1>
    struct Operation {
      T0 operator()(T0 left, T1 right) const {
        return std::max(left, right);
      }
    };
  };

  /**
   * Constructs a FunctionExpression representing the min function.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing the min function.
   */
  inline FunctionExpression min(
      const Expression& left, const Expression& right) {
    return FunctionExpression(MIN_NAME, left.get_type(), {left, right});
  }

  /**
   * Translates min expressions.
   * @tparam ValueTypes The list of types that support min.
   */
  template<typename ValueTypes>
  struct MinExpressionTranslator {
    using type = MaxExpressionTranslator<ValueTypes>::type;

    template<typename T0, typename T1>
    struct Operation {
      T0 operator()(T0 left, T1 right) const {
        return std::min(left, right);
      }
    };
  };
}

#endif
