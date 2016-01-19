#ifndef BEAM_MATH_HPP
#define BEAM_MATH_HPP
#include <cstdint>
#include <type_traits>
#include "Beam/Utilities/MathDetails.hpp"

namespace Beam {

  //! Returns a power of 10.
  /*!
    \param exponent The exponent.
    \return 10 raised to the <i>exponent</i>.
  */
  template<typename T>
  std::uint64_t PowerOfTen(T exponent) {
    std::uint64_t result = 1;
    for(T i = 0; i < exponent; ++i) {
      result *= 10;
    }
    return result;
  }

  //! Returns the ceiling of a number to a point.
  /*!
    \param value The value to ceil.
    \param point The point to ceil to.
    \return The <i>value</i> ceiled to the specified <i>point</i>.
  */
  inline double Ceil(double value, int point = 1) {
    double d = value / point;
    d = std::ceil(d);
    return d * point;
  }

  //! Returns the ceiling of a number to a point.
  /*!
    \param value The value to ceil.
    \param point The point to ceil to.
    \return The <i>value</i> ceiled to the specified <i>point</i>.
  */
  template<typename T>
  typename std::enable_if<std::is_integral<T>::value, T>::type Ceil(T value,
      int point = 1) {
    double d = static_cast<double>(value) / point;
    d = std::ceil(d) + 0.5;
    return static_cast<int>(d) * point;
  }

  //! Returns the floor of a number to a point.
  /*!
    \param value The value to floor.
    \param point The point to floor to.
    \return The <i>value</i> floored to the specified <i>point</i>.
  */
  inline double Floor(double value, int point = 1) {
    double d = value / point;
    d = std::floor(d);
    return d * point;
  }

  //! Returns the floor of a number to a point.
  /*!
    \param value The value to floor.
    \param point The point to floor to.
    \return The <i>value</i> floored to the specified <i>point</i>.
  */
  template<typename T>
  typename std::enable_if<std::is_integral<T>::value, T>::type Floor(T value,
      int point = 1) {
    double d = static_cast<double>(value) / point;
    d = std::floor(d) + 0.5;
    return static_cast<int>(d) * point;
  }

  //! Rounds a number to a point.
  /*!
    \param value The value to round.
    \param point The point to round to.
    \return The <i>value</i> rounded to the specified <i>point</i>.
  */
  inline double Round(double value, int point = 1) {
    double d = value / point;
    d = Details::RoundDouble(d);
    return d * point;
  }

  //! Rounds a number to a point.
  /*!
    \param value The value to round.
    \param point The point to round to.
    \return The <i>value</i> rounded to the specified <i>point</i>.
  */
  template<typename T>
  typename std::enable_if<std::is_integral<T>::value, T>::type Round(T value,
      int point = 1) {
    double d = static_cast<double>(value) / point;
    d = Details::RoundDouble(d) + 0.5;
    return static_cast<int>(d) * point;
  }
}

#endif
