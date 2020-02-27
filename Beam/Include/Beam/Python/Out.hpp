#ifndef BEAM_PYTHON_OUT_HPP
#define BEAM_PYTHON_OUT_HPP
#include <pybind11/pybind11.h>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {

  /**
   * Implements a type caster for Beam::Out types.
   * @param <T> The type of Out to cast.
   */
  template<typename T>
  struct OutTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using Converter = pybind11::detail::make_caster<typename Type::Type>;
    static constexpr auto name = pybind11::detail::_("Out[") + Converter::name +
      pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  template<typename V>
  pybind11::handle OutTypeCaster<T>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      typename Type::Type>::policy(policy);
    return Converter::cast(*std::forward<V>(value), policy, parent);
  }

  template<typename T>
  bool OutTypeCaster<T>::load(pybind11::handle source, bool convert) {
    auto caster = Converter();
    if(!caster.load(source, convert)) {
      return false;
    }
    m_value.emplace(Store(pybind11::detail::cast_op<typename Type::Type&&>(
      std::move(caster))));
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::Out<T>> :
    Beam::Python::OutTypeCaster<Beam::Out<T>> {};
}

#endif
