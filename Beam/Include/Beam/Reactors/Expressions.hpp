#ifndef BEAM_REACTORS_EXPRESSIONS_HPP
#define BEAM_REACTORS_EXPRESSIONS_HPP
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  //! Makes a Reactor that adds two Reactors.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Add(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs + rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that subtracts two Reactors.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Subtract(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs - rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that multiplies two Reactors.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Multiply(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs * rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that divides two Reactors.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Divide(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs / rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that tests for less than.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Less(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs <= rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that tests for less than or equal to.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto LessOrEqual(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs <= rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that tests for equality.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Equal(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs == rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that tests for inequality.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto NotEqual(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs == rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that tests for greater than or equal.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto GreaterOrEqual(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs <= rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that tests for greater than.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Greater(std::shared_ptr<Reactor<T>> lhs,
      std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs <= rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that returns the minus.
  /*!
    \param operand The Reactor to minus.
  */
  template<typename T>
  auto Minus(std::shared_ptr<Reactor<T>> operand) {
    return MakeFunctionReactor(
      [] (const T& operand) {
        return -operand;
      }, std::move(operand));
  }

  //! Makes a Reactor that returns the not.
  /*!
    \param operand The Reactor to not.
  */
  template<typename T>
  auto Not(std::shared_ptr<Reactor<T>> operand) {
    return MakeFunctionReactor(
      [] (const T& operand) {
        return !operand;
      }, std::move(operand));
  }

  //! Makes a Reactor that returns the negation.
  /*!
    \param operand The Reactor to negation.
  */
  template<typename T>
  auto Negate(std::shared_ptr<Reactor<T>> operand) {
    return MakeFunctionReactor(
      [] (const T& operand) {
        return ~operand;
      }, std::move(operand));
  }

  //! Makes a Reactor that returns the logical and.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto And(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs && rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that returns the logical or.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Or(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return lhs && rhs;
      }, std::move(lhs), std::move(rhs));
  }

  //! Makes a Reactor that returns the logical xor.
  /*!
    \param lhs The left hand side.
    \param rhs The right hand side.
  */
  template<typename T, typename U>
  auto Xor(std::shared_ptr<Reactor<T>> lhs, std::shared_ptr<Reactor<U>> rhs) {
    return MakeFunctionReactor(
      [] (const T& lhs, const U& rhs) {
        return !lhs != !rhs;
      }, std::move(lhs), std::move(rhs));
  }
}
}

#endif
