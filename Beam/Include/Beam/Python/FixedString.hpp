#ifndef BEAM_PYTHON_FIXED_STRING_HPP
#define BEAM_PYTHON_FIXED_STRING_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam::Python {
namespace Details {
  template<unsigned... D>
  struct ToChars {
    static constexpr char value[sizeof...(D) + 1] = {('0' + D)..., 0};
  };

  template<unsigned R, unsigned... D>
  struct Explode : Explode<R / 10, R % 10, D...> {};

  template<unsigned... D>
  struct Explode<0, D...> : ToChars<D...> {};

  template<unsigned N>
  struct FixedStringName : Explode<N> {};
}

  /**
   * Implements a type caster for FixedStrings.
   * @param <N> The size of the FixedString to cast.
   */
  template<typename T>
  struct FixedStringTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static constexpr auto name = pybind11::detail::_("FixedString[") +
      pybind11::detail::_(Details::FixedStringName<Type::SIZE>::value) +
      pybind11::detail::_("]");
    static pybind11::handle cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle FixedStringTypeCaster<T>::cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return pybind11::cast(value.GetData()).release();
  }

  template<typename T>
  bool FixedStringTypeCaster<T>::load(pybind11::handle source, bool) {
    if(!PyUnicode_Check(source.ptr())) {
      return false;
    }
    m_value.emplace(PyUnicode_AsUTF8(source.ptr()));
    return true;
  }
}

namespace pybind11::detail {
  template<std::size_t N>
  struct type_caster<Beam::FixedString<N>> :
    Beam::Python::FixedStringTypeCaster<Beam::FixedString<N>> {};
}

#endif
