#ifndef BEAM_PYTHON_VARIANT_HPP
#define BEAM_PYTHON_VARIANT_HPP
#include <boost/mpl/for_each.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Beam/Python/BasicTypeCaster.hpp"

namespace pybind11::detail {
  template<typename... T>
  struct type_caster<boost::variant<T...>> :
      Beam::Python::BasicTypeCaster<boost::variant<T...>> {
    using Type = boost::variant<T...>;
    static pybind11::handle cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using Beam::Python::BasicTypeCaster<Type>::m_value;
  };

  template<typename... T>
  pybind11::handle type_caster<boost::variant<T...>>::cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return boost::apply_visitor(
      [] (auto&& value) {
        return pybind11::cast(std::forward<decltype(value)>(value));
      }, value);
  }

  template<typename... T>
  bool type_caster<boost::variant<T...>>::load(pybind11::handle source, bool) {
    auto is_converted = false;
    boost::mpl::for_each<typename boost::variant<T...>::types>(
      [&] (auto&& unused) {
        using U = std::decay_t<decltype(unused)>();
        if(is_converted) {
          return;
        }
        if(!pybind11::isinstance<U>(source)) {
          return;
        }
        m_value.emplace(source.cast<U>());
        is_converted = true;
      });
    return is_converted;
  }
}

#endif
