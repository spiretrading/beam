#ifndef BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#define BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>
#include <boost/mpl/back.hpp>
#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/front_inserter.hpp>
#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

namespace Beam::Queries {
namespace Details {
  template<typename Q,
    typename = decltype(std::declval<Q>() < std::declval<Q>())>
  std::true_type HasLessThanTest(const Q&);
  std::false_type HasLessThanTest(...);

  template<typename T1>
  struct MakePairs {
    template <typename T2>
    struct apply {
      using type = boost::mpl::vector<T1, T2>;
    };
  };

  template<typename T, template<typename> typename HasOperation>
  struct Signatures {
    using type = typename boost::mpl::copy_if<
      typename boost::mpl::fold<T, boost::mpl::list<>,
        boost::mpl::transform<T, MakePairs<boost::mpl::_2>,
          boost::mpl::front_inserter<boost::mpl::_1>>>::type,
      HasOperation<boost::mpl::_1>,
      boost::mpl::front_inserter<boost::mpl::list<>>>::type;
  };
}

  /** The name used for the addition function. */
  inline const std::string ADDITION_NAME = "+";

  /** The name used for the subtraction function. */
  inline const std::string SUBTRACTION_NAME = "-";

  /** The name used for the multiplication function. */
  inline const std::string MULTIPLICATION_NAME = "*";

  /** The name used for the division function. */
  inline const std::string DIVISION_NAME = "/";

  /** The name used for the less function. */
  inline const std::string LESS_NAME = "<";

  /** The name used for the less or equals function. */
  inline const std::string LESS_EQUALS_NAME = "<=";

  /** The name used for the equals function. */
  inline const std::string EQUALS_NAME = "==";

  /** The name used for the inequality function. */
  inline const std::string NOT_EQUALS_NAME = "!=";

  /** The name used for the greater or equals function. */
  inline const std::string GREATER_EQUALS_NAME = ">=";

  /** The name used for the greater function. */
  inline const std::string GREATER_NAME = ">";

  /** The name used for the max function. */
  inline const std::string MAX_NAME = "max";

  /** The name used for the min function. */
  inline const std::string MIN_NAME = "min";

  /**
   * Constructs a FunctionExpression representing addition.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing addition.
   */
  inline FunctionExpression MakeAdditionExpression(
      const Expression& left, const Expression& right) {
    auto type = [&] () -> DataType {
      if(left->GetType() == IntType() && right->GetType() == DecimalType()) {
        return DecimalType::GetInstance();
      }
      return left->GetType();
    }();
    return FunctionExpression(ADDITION_NAME, type, {left, right});
  }

  inline FunctionExpression operator +(
      const Expression& left, const Expression& right) {
    return MakeAdditionExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing subtraction.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing subtraction.
   */
  inline FunctionExpression MakeSubtractionExpression(
      const Expression& left, const Expression& right) {
    auto type = [&] () -> DataType {
      if(left->GetType() == IntType() && right->GetType() == DecimalType()) {
        return DecimalType::GetInstance();
      }
      return left->GetType();
    }();
    return FunctionExpression(SUBTRACTION_NAME, type, {left, right});
  }

  inline FunctionExpression operator -(
      const Expression& left, const Expression& right) {
    return MakeSubtractionExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing multiplication.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing multiplication.
   */
  inline FunctionExpression MakeMultiplicationExpression(
      const Expression& left, const Expression& right) {
    auto type = [&] () -> DataType {
      if(right->GetType() == IntType() && left->GetType() == DecimalType()) {
        return DecimalType::GetInstance();
      }
      return right->GetType();
    }();
    return FunctionExpression(MULTIPLICATION_NAME, type, {left, right});
  }

  inline FunctionExpression operator *(
      const Expression& left, const Expression& right) {
    return MakeMultiplicationExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing division.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing division.
   */
  inline FunctionExpression MakeDivisionExpression(
      const Expression& left, const Expression& right) {
    auto type = [&] () -> DataType {
      if(left->GetType() == IntType() && right->GetType() == DecimalType()) {
        return DecimalType::GetInstance();
      }
      return left->GetType();
    }();
    return FunctionExpression(DIVISION_NAME, type, {left, right});
  }

  inline FunctionExpression operator /(
      const Expression& left, const Expression& right) {
    return MakeDivisionExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing less.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing less.
   */
  inline FunctionExpression MakeLessExpression(
      const Expression& left, const Expression& right) {
    return FunctionExpression(LESS_NAME, BoolType(), {left, right});
  }

  inline FunctionExpression operator <(
      const Expression& left, const Expression& right) {
    return MakeLessExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing less than or equals.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing less than or equals.
   */
  inline FunctionExpression MakeLessOrEqualsExpression(
      const Expression& left, const Expression& right) {
    return FunctionExpression(LESS_EQUALS_NAME, BoolType(), {left, right});
  }

  inline FunctionExpression operator <=(
      const Expression& left, const Expression& right) {
    return MakeLessOrEqualsExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing equality.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing equality.
   */
  inline FunctionExpression MakeEqualsExpression(
      const Expression& left, const Expression& right) {
    return FunctionExpression(EQUALS_NAME, BoolType(), {left, right});
  }

  inline FunctionExpression operator ==(
      const Expression& left, const Expression& right) {
    return MakeEqualsExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing inequality.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing inequality.
   */
  inline FunctionExpression MakeNotEqualsExpression(
      const Expression& left, const Expression& right) {
    return FunctionExpression(NOT_EQUALS_NAME, BoolType(), {left, right});
  }

  inline FunctionExpression operator !=(
      const Expression& left, const Expression& right) {
    return MakeNotEqualsExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing greater or equals.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing greater or equals.
   */
  inline FunctionExpression MakeGreaterEqualsExpression(
      const Expression& left, const Expression& right) {
    return FunctionExpression(GREATER_EQUALS_NAME, BoolType(), {left, right});
  }

  inline FunctionExpression operator >=(
      const Expression& left, const Expression& right) {
    return MakeGreaterEqualsExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing greater.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing greater.
   */
  inline FunctionExpression MakeGreaterExpression(
      const Expression& left, const Expression& right) {
    return FunctionExpression(GREATER_NAME, BoolType(), {left, right});
  }

  inline FunctionExpression operator >(
      const Expression& left, const Expression& right) {
    return MakeGreaterExpression(left, right);
  }

  /**
   * Constructs a FunctionExpression representing the max function.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing the max function.
   */
  inline FunctionExpression MakeMaxExpression(const Expression& left,
      const Expression& right) {
    return FunctionExpression(MAX_NAME, left->GetType(), {left, right});
  }

  /**
   * Constructs a FunctionExpression representing the min function.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing the min function.
   */
  inline FunctionExpression MakeMinExpression(const Expression& left,
      const Expression& right) {
    return FunctionExpression(MIN_NAME, left->GetType(), {left, right});
  }

  /**
   * Tests if a type has a less than operator.
   * @param <T> The type to test.
   */
  template<typename T>
  struct HasLessThan :
    std::is_same<decltype(Details::HasLessThanTest(std::declval<T>())),
    std::true_type>::type {};

  /** Contains the meta-data needed to translate an addition Expression. */
  template<typename ValueTypes>
  struct AdditionExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left + right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        { left + right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a subtraction Expression. */
  template<typename ValueTypes>
  struct SubtractionExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left - right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        { left - right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a multiplication Expression. */
  template<typename ValueTypes>
  struct MultiplicationExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left * right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires !std::is_same_v<typename boost::mpl::front<T>::type,
          boost::posix_time::time_duration>;
        requires !std::is_same_v<
          typename boost::mpl::back<T>::type, boost::posix_time::time_duration>;
        { left * right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a division Expression. */
  template<typename ValueTypes>
  struct DivisionExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left / right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires !std::is_same_v<
          typename boost::mpl::front<T>::type, bool>;
        requires !std::is_same_v<
          typename boost::mpl::back<T>::type, bool>;
        requires !std::is_same_v<typename boost::mpl::front<T>::type,
          boost::posix_time::time_duration>;
        requires !std::is_same_v<
          typename boost::mpl::back<T>::type, boost::posix_time::time_duration>;
        { left / right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a less Expression. */
  template<typename ValueTypes>
  struct LessExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left < right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires !std::is_same_v<typename boost::mpl::front<T>::type, bool>;
        requires !std::is_same_v<typename boost::mpl::back<T>::type, bool>;
        { left < right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a less or equals Expression. */
  template<typename ValueTypes>
  struct LessEqualsExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left <= right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires !std::is_same_v<typename boost::mpl::front<T>::type, bool>;
        requires !std::is_same_v<typename boost::mpl::back<T>::type, bool>;
        { left <= right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate an equals Expression. */
  template<typename ValueTypes>
  struct EqualsExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left == right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires std::is_same_v<typename boost::mpl::front<T>::type,
          typename boost::mpl::back<T>::type>;
        { left == right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a not equals Expression. */
  template<typename ValueTypes>
  struct NotEqualsExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left != right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires std::is_same_v<typename boost::mpl::front<T>::type,
          typename boost::mpl::back<T>::type>;
        { left != right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /**
   * Contains the meta-data needed to translate a greater or equals Expression.
   */
  template<typename ValueTypes>
  struct GreaterEqualsExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left >= right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires !std::is_same_v<typename boost::mpl::front<T>::type, bool>;
        requires !std::is_same_v<typename boost::mpl::back<T>::type, bool>;
        { left >= right };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a greater Expression. */
  template<typename ValueTypes>
  struct GreaterExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left > right;
      }
    };

    template<typename T>
    struct HasOperation : std::bool_constant<
      requires(const typename boost::mpl::front<T>::type& left,
          const typename boost::mpl::back<T>::type& right) {
        requires !std::is_same_v<typename boost::mpl::front<T>::type, bool>;
        requires !std::is_same_v<typename boost::mpl::back<T>::type, bool>;
        { (left > right) };
      }> {};

    using SupportedTypes =
      typename Details::Signatures<ValueTypes, HasOperation>::type;
  };

  /** Contains the meta-data needed to translate a max Expression. */
  template<typename ValueTypes>
  struct MaxExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      T0 operator()(T0 left, T1 right) const {
        return std::max(left, right);
      }
    };

    template<typename T>
    struct MakeSignature {
      using type = typename boost::mpl::vector<T, T, T>::type;
    };

    using ComparableTypes = typename boost::mpl::copy_if<ValueTypes,
      HasLessThan<boost::mpl::placeholders::_1>,
      boost::mpl::front_inserter<boost::mpl::list<>>>::type;

    using SupportedTypes = typename boost::mpl::transform<ComparableTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type;
  };

  /** Contains the meta-data needed to translate a min Expression. */
  template<typename ValueTypes>
  struct MinExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      T0 operator()(T0 left, T1 right) const {
        return std::min(left, right);
      }
    };

    template<typename T>
    struct MakeSignature {
      using type = typename boost::mpl::vector<T, T, T>::type;
    };

    using ComparableTypes = typename boost::mpl::copy_if<ValueTypes,
      HasLessThan<boost::mpl::placeholders::_1>,
      boost::mpl::front_inserter<boost::mpl::list<>>>::type;

    using SupportedTypes = typename boost::mpl::transform<ComparableTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type;
  };
}

#endif
