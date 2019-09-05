#ifndef BEAM_PYTHON_ENUM_HPP
#define BEAM_PYTHON_ENUM_HPP
#include <pybind11/pybind11.h>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {

  /**
   * Implements a type caster for Beam::Enum types.
   * @param <T> The type of enum to cast.
   */
  template<typename T>
  struct EnumTypeCaster : BasicTypeCaster<T> {

    /** The enum being cast. */
    using Enum = T;
    static handle cast(Enum value, return_value_policy policy, handle parent);
    bool load(handle source, bool);
  };

  template<typename T>
  handle EnumTypeCaster<T>::cast(Enum value, return_value_policy policy,
      handle parent) {
    return pybind11::cast(static_cast<typename Enum::Type>(value));
  }

  template<typename T>
  bool EnumTypeCaster<T>::load(handle source, bool) {
    try {
      m_value.emplace(source.cast<typename Enum::Type>());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

#endif
