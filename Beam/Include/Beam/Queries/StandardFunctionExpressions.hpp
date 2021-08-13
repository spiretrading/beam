#ifndef BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#define BEAM_STANDARD_FUNCTION_EXPRESSIONS_HPP
#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>
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
}

  /** The name used for the addition function. */
  inline const std::string ADDITION_NAME = "+";

  /** The name used for the equals function. */
  inline const std::string EQUALS_NAME = "==";

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
  inline FunctionExpression MakeAdditionExpression(const Expression& left,
      const Expression& right) {
    auto parameters = std::vector<Expression>();
    parameters.push_back(left);
    parameters.push_back(right);
    auto expression = FunctionExpression(ADDITION_NAME, left->GetType(),
      std::move(parameters));
    return expression;
  }

  /**
   * Constructs a FunctionExpression representing equality.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing equality.
   */
  inline FunctionExpression MakeEqualsExpression(const Expression& left,
      const Expression& right) {
    auto parameters = std::vector<Expression>();
    parameters.push_back(left);
    parameters.push_back(right);
    auto expression = FunctionExpression(EQUALS_NAME, BoolType(),
      std::move(parameters));
    return expression;
  }

  /**
   * Constructs a FunctionExpression representing the max function.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing the max function.
   */
  inline FunctionExpression MakeMaxExpression(const Expression& left,
      const Expression& right) {
    auto parameters = std::vector<Expression>();
    parameters.push_back(left);
    parameters.push_back(right);
    auto expression = FunctionExpression(MAX_NAME, left->GetType(),
      std::move(parameters));
    return expression;
  }

  /**
   * Constructs a FunctionExpression representing the min function.
   * @param The left hand side of the expression.
   * @param The right hand side of the expression.
   * @return A FunctionExpression representing the min function.
   */
  inline FunctionExpression MakeMinExpression(const Expression& left,
      const Expression& right) {
    auto parameters = std::vector<Expression>();
    parameters.push_back(left);
    parameters.push_back(right);
    auto expression = FunctionExpression(MIN_NAME, left->GetType(),
      std::move(parameters));
    return expression;
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
  struct AdditionExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left + right;
      }
    };

    using SupportedTypes = boost::mpl::list<boost::mpl::vector<int, int>,
      boost::mpl::vector<double, double>,
      boost::mpl::vector<boost::posix_time::time_duration,
        boost::posix_time::time_duration>,
      boost::mpl::vector<boost::posix_time::ptime,
        boost::posix_time::time_duration>>;
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
    struct MakeSignature {
      using type = typename boost::mpl::vector<T, T, bool>::type;
    };

    using SupportedTypes = typename boost::mpl::transform<ValueTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type;
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
