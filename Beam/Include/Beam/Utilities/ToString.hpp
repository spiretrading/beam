#ifndef BEAM_TOSTRING_HPP
#define BEAM_TOSTRING_HPP
#include <string>
#include "Beam/Utilities/Convert.hpp"

namespace Beam {

  //! Specializes the Convert function to strings.
  /*!
    \param value The value to convert.
    \return The string representation of the <i>value</i>.
  */
  template<typename T>
  std::string ToString(const T& value) {
    return Convert<std::string>(value);
  }

  //! Specializes the Convert function from strings.
  /*!
    \param value The string to convert.
    \return The string converted to the specified type.
  */
  template<typename T>
  T FromString(const std::string& value) {
    return Convert<T>(value);
  }
}

#endif
