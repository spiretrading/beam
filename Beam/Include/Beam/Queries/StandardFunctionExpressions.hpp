#ifndef BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#define BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#include <concepts>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <boost/callable_traits/return_type.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/mp11.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/QueryTypes.hpp"

namespace Beam {
namespace Details {
  template<typename T0, typename T1, typename T, typename U>
  inline constexpr auto is_pair_v =
    (std::is_same_v<T0, T> && std::is_same_v<T1, U>) ||
    (std::is_same_v<T0, U> && std::is_same_v<T1, T>);

  template<typename T0, typename T1>
  inline constexpr auto is_mixed_integer_v =
    is_pair_v<T0, T1, int, std::uint64_t>;

  template<typename TypeList, std::size_t I, std::size_t J, std::size_t Size,
    template<typename, typename> class HasOperation>
  struct generate_pairs_impl {
    using current_type1 = boost::mp11::mp_at_c<TypeList, I>;
    using current_type2 = boost::mp11::mp_at_c<TypeList, J>;
    using next_pairs = typename std::conditional_t<
      J + 1 < Size, generate_pairs_impl<TypeList, I, J + 1, Size, HasOperation>,
      std::conditional_t<I + 1 < Size,
        generate_pairs_impl<TypeList, I + 1, 0, Size, HasOperation>,
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
    using type = std::conditional_t<size == 0, boost::mp11::mp_list<>,
      typename generate_pairs_impl<TypeList, 0, 0, size, HasOperation>::type>;
  };

  template<typename T, template<typename, typename> class HasOperation>
  struct make_parameter_list {
    using type = typename generate_pairs<T, HasOperation>::type;
  };

  template<typename F, typename T0, typename T1>
  using operation_result_t =
    std::remove_cvref_t<boost::callable_traits::return_type_t<
      typename F::template Operation<T0, T1>>>;

  using PromotionKey =
    std::tuple<std::string, std::type_index, std::type_index>;

  inline auto& get_promotions() {
    static auto promotions = std::map<PromotionKey, std::type_index>();
    return promotions;
  }

  template<typename F>
  void register_promotion(const std::string& name) {
    boost::mp11::mp_for_each<typename F::type>([&] (auto operands) {
      using Left = boost::mp11::mp_first<decltype(operands)>;
      using Right = boost::mp11::mp_second<decltype(operands)>;
      get_promotions().insert_or_assign(
        PromotionKey(name, typeid(Left), typeid(Right)),
        std::type_index(typeid(operation_result_t<F, Left, Right>)));
    });
  }

  inline std::type_index get_promotion(const std::string& name,
      const Expression& left, const Expression& right) {
    auto& promotions = get_promotions();
    auto i = promotions.find(
      PromotionKey(name, left.get_type(), right.get_type()));
    if(i == promotions.end()) {
      return left.get_type();
    }
    return i->second;
  }
}

  /**
   * Specifies whether two types may be used as the operands of a single
   * operation.
   * @tparam T The type of the left hand operand.
   * @tparam U The type of the right hand operand.
   */
  template<typename T, typename U>
  struct is_compatible_operand : std::false_type {};

  template<typename T>
  struct is_compatible_operand<T, T> : std::true_type {};

  template<>
  struct is_compatible_operand<int, double> : std::true_type {};

  template<>
  struct is_compatible_operand<double, int> : std::true_type {};

  template<>
  struct is_compatible_operand<int, std::uint64_t> : std::true_type {};

  template<>
  struct is_compatible_operand<std::uint64_t, int> : std::true_type {};

  template<>
  struct is_compatible_operand<boost::posix_time::ptime,
    boost::posix_time::time_duration> : std::true_type {};

  /**
   * Whether two types may be used as the operands of a single operation.
   * @tparam T The type of the left hand operand.
   * @tparam U The type of the right hand operand.
   */
  template<typename T, typename U>
  inline constexpr auto is_compatible_operand_v =
    is_compatible_operand<T, U>::value;

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
    return FunctionExpression(ADDITION_NAME,
      Details::get_promotion(ADDITION_NAME, left, right), {left, right});
  }

  /**
   * Constructs a FunctionExpression representing addition.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing addition.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator +(const Expression& left, const T& right) {
    return left + ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing addition.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing addition.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator +(const T& left, const Expression& right) {
    return ConstantExpression(left) + right;
  }

  /**
   * Translates addition expressions.
   * @tparam ValueTypes The list of types that support addition.
   */
  template<typename ValueTypes>
  struct AdditionExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires is_compatible_operand_v<A1, A2>;
      requires !Details::is_mixed_integer_v<A1, A2>;
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
    return FunctionExpression(SUBTRACTION_NAME,
      Details::get_promotion(SUBTRACTION_NAME, left, right), {left, right});
  }

  /**
   * Constructs a FunctionExpression representing subtraction.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing subtraction.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator -(const Expression& left, const T& right) {
    return left - ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing subtraction.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing subtraction.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator -(const T& left, const Expression& right) {
    return ConstantExpression(left) - right;
  }

  /**
   * Translates subtraction expressions.
   * @tparam ValueTypes The list of types that support subtraction.
   */
  template<typename ValueTypes>
  struct SubtractionExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires is_compatible_operand_v<A1, A2>;
      requires !Details::is_mixed_integer_v<A1, A2>;
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
    return FunctionExpression(MULTIPLICATION_NAME,
      Details::get_promotion(MULTIPLICATION_NAME, left, right),
      {left, right});
  }

  /**
   * Constructs a FunctionExpression representing multiplication.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing multiplication.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator *(const Expression& left, const T& right) {
    return left * ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing multiplication.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing multiplication.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator *(const T& left, const Expression& right) {
    return ConstantExpression(left) * right;
  }

  /**
   * Translates multiplication expressions.
   * @tparam ValueTypes The list of types that support multiplication.
   */
  template<typename ValueTypes>
  struct MultiplicationExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires is_compatible_operand_v<A1, A2>;
      requires !Details::is_mixed_integer_v<A1, A2>;
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
    return FunctionExpression(DIVISION_NAME,
      Details::get_promotion(DIVISION_NAME, left, right), {left, right});
  }

  /**
   * Constructs a FunctionExpression representing division.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing division.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator /(const Expression& left, const T& right) {
    return left / ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing division.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing division.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator /(const T& left, const Expression& right) {
    return ConstantExpression(left) / right;
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
      requires is_compatible_operand_v<A1, A2>;
      requires !Details::is_mixed_integer_v<A1, A2>;
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
   * Constructs a FunctionExpression representing less.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing less.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator <(const Expression& left, const T& right) {
    return left < ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing less.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing less.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator <(const T& left, const Expression& right) {
    return ConstantExpression(left) < right;
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
      requires is_compatible_operand_v<A1, A2>;
      { std::declval<A1>() < std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        if constexpr(Details::is_mixed_integer_v<T0, T1>) {
          return std::cmp_less(left, right);
        } else {
          return left < right;
        }
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
   * Constructs a FunctionExpression representing less than or equals.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing less than or equals.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator <=(const Expression& left, const T& right) {
    return left <= ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing less than or equals.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing less than or equals.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator <=(const T& left, const Expression& right) {
    return ConstantExpression(left) <= right;
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
      requires is_compatible_operand_v<A1, A2>;
      { std::declval<A1>() <= std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        if constexpr(Details::is_mixed_integer_v<T0, T1>) {
          return std::cmp_less_equal(left, right);
        } else {
          return left <= right;
        }
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
   * Constructs a FunctionExpression representing equality.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing equality.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator ==(const Expression& left, const T& right) {
    return left == ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing equality.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing equality.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator ==(const T& left, const Expression& right) {
    return ConstantExpression(left) == right;
  }

  /**
   * Translates equals expressions.
   * @tparam ValueTypes The list of types that support equals.
   */
  template<typename ValueTypes>
  struct EqualsExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires is_compatible_operand_v<A1, A2>;
      { std::declval<A1>() == std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        if constexpr(Details::is_mixed_integer_v<T0, T1>) {
          return std::cmp_equal(left, right);
        } else {
          return left == right;
        }
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
   * Constructs a FunctionExpression representing inequality.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing inequality.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator !=(const Expression& left, const T& right) {
    return left != ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing inequality.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing inequality.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator !=(const T& left, const Expression& right) {
    return ConstantExpression(left) != right;
  }

  /**
   * Translates not equals expressions.
   * @tparam ValueTypes The list of types that support not equals.
   */
  template<typename ValueTypes>
  struct NotEqualsExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires is_compatible_operand_v<A1, A2>;
      { std::declval<A1>() != std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        if constexpr(Details::is_mixed_integer_v<T0, T1>) {
          return std::cmp_not_equal(left, right);
        } else {
          return left != right;
        }
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
   * Constructs a FunctionExpression representing greater or equals.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing greater or equals.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator >=(const Expression& left, const T& right) {
    return left >= ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing greater or equals.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing greater or equals.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator >=(const T& left, const Expression& right) {
    return ConstantExpression(left) >= right;
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
      requires is_compatible_operand_v<A1, A2>;
      { std::declval<A1>() >= std::declval<A2>() };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        if constexpr(Details::is_mixed_integer_v<T0, T1>) {
          return std::cmp_greater_equal(left, right);
        } else {
          return left >= right;
        }
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
   * Constructs a FunctionExpression representing greater.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing greater.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator >(const Expression& left, const T& right) {
    return left > ConstantExpression(right);
  }

  /**
   * Constructs a FunctionExpression representing greater.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing greater.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression operator >(const T& left, const Expression& right) {
    return ConstantExpression(left) > right;
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
      requires is_compatible_operand_v<A1, A2>;
      { (std::declval<A1>() > std::declval<A2>()) };
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      decltype(auto) operator()(T0 left, T1 right) const {
        if constexpr(Details::is_mixed_integer_v<T0, T1>) {
          return std::cmp_greater(left, right);
        } else {
          return left > right;
        }
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
    return FunctionExpression(MAX_NAME,
      Details::get_promotion(MAX_NAME, left, right), {left, right});
  }

  /**
   * Constructs a FunctionExpression representing the max function.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing the max function.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression max(const Expression& left, const T& right) {
    return max(left, ConstantExpression(right));
  }

  /**
   * Constructs a FunctionExpression representing the max function.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing the max function.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression max(const T& left, const Expression& right) {
    return max(ConstantExpression(left), right);
  }

  /**
   * Translates max expressions.
   * @tparam ValueTypes The list of types that support max.
   */
  template<typename ValueTypes>
  struct MaxExpressionTranslator {
    template<typename A1, typename A2>
    struct has_operation : std::bool_constant<requires {
      requires is_compatible_operand_v<A1, A2>;
      requires !Details::is_mixed_integer_v<A1, A2>;
      typename std::common_type_t<A1, A2>;
      { std::declval<std::common_type_t<A1, A2>>() <
        std::declval<std::common_type_t<A1, A2>>() } ->
          std::convertible_to<bool>;
    }> {};

    using type = Details::make_parameter_list<ValueTypes, has_operation>::type;

    template<typename T0, typename T1>
    struct Operation {
      std::common_type_t<T0, T1> operator()(T0 left, T1 right) const {
        return std::max<std::common_type_t<T0, T1>>(left, right);
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
    return FunctionExpression(MIN_NAME,
      Details::get_promotion(MIN_NAME, left, right), {left, right});
  }

  /**
   * Constructs a FunctionExpression representing the min function.
   * @param left The left hand side of the expression.
   * @param right The right hand side value.
   * @return A FunctionExpression representing the min function.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression min(const Expression& left, const T& right) {
    return min(left, ConstantExpression(right));
  }

  /**
   * Constructs a FunctionExpression representing the min function.
   * @param left The left hand side value.
   * @param right The right hand side of the expression.
   * @return A FunctionExpression representing the min function.
   */
  template<typename T> requires(!std::derived_from<T, VirtualExpression>)
  FunctionExpression min(const T& left, const Expression& right) {
    return min(ConstantExpression(left), right);
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
      std::common_type_t<T0, T1> operator()(T0 left, T1 right) const {
        return std::min<std::common_type_t<T0, T1>>(left, right);
      }
    };
  };

  /**
   * Registers the type that each standard function produces for the operands
   * it accepts.
   * @tparam Q The types whose operations are registered.
   * @return <code>true</code>.
   */
  template<typename Q>
  bool register_promotions() {
    using NativeTypes = typename Q::NativeTypes;
    using ComparableTypes = typename Q::ComparableTypes;
    Details::register_promotion<AdditionExpressionTranslator<NativeTypes>>(
      ADDITION_NAME);
    Details::register_promotion<SubtractionExpressionTranslator<NativeTypes>>(
      SUBTRACTION_NAME);
    Details::register_promotion<
      MultiplicationExpressionTranslator<NativeTypes>>(MULTIPLICATION_NAME);
    Details::register_promotion<DivisionExpressionTranslator<NativeTypes>>(
      DIVISION_NAME);
    Details::register_promotion<MaxExpressionTranslator<ComparableTypes>>(
      MAX_NAME);
    Details::register_promotion<MinExpressionTranslator<ComparableTypes>>(
      MIN_NAME);
    return true;
  }

namespace Details {
  inline const auto STANDARD_PROMOTIONS = register_promotions<QueryTypes>();
}
}

#endif
