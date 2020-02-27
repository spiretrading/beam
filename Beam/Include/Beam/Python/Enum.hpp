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
    using Type = T;
    using Converter = pybind11::detail::make_caster<typename Type::Type>;
    static constexpr auto name = pybind11::detail::_("Enum[") +
      Converter::name + pybind11::detail::_("]");
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle EnumTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      typename Type::Type>::policy(policy);
    return Converter::cast(static_cast<typename Type::Type>(value), policy,
      parent);
  }

  template<typename T>
  bool EnumTypeCaster<T>::load(pybind11::handle source, bool convert) {
    auto caster = Converter();
    if(!caster.load(source, convert)) {
      return false;
    }
    m_value.emplace(pybind11::detail::cast_op<typename Type::Type&&>(
      std::move(caster)));
    return true;
  }
}

namespace pybind11::detail {
  template<typename T, std::size_t N>
  struct type_caster<Beam::Enum<T, N>> : Beam::Python::EnumTypeCaster<
    Beam::Enum<T, N>> {};
}

#endif
