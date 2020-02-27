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
    static constexpr auto name = pybind11::detail::_("Variant[") +
      pybind11::detail::concat(pybind11::detail::make_caster<T>::name...) +
      pybind11::detail::_("]");
    template<typename V>
    static pybind11::handle cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool convert);
    using Beam::Python::BasicTypeCaster<Type>::m_value;
  };

  template<typename... T>
  template<typename V>
  pybind11::handle type_caster<boost::variant<T...>>::cast(V&& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return boost::apply_visitor(
      [&] (auto&& value) {
        using U = std::decay_t<decltype(value)>;
        policy = pybind11::detail::return_value_policy_override<U>::policy(
          policy);
        return pybind11::detail::make_caster<U>::cast(
          std::forward<decltype(value)>(value), policy, parent);
      }, std::forward<V>(value));
  }

  template<typename... T>
  bool type_caster<boost::variant<T...>>::load(pybind11::handle source,
      bool convert) {
    auto is_converted = false;
    boost::mpl::for_each<typename boost::variant<T...>::types>(
      [&] (auto&& unused) {
        using U = std::decay_t<decltype(unused)>;
        if(is_converted) {
          return;
        }
        auto caster = pybind11::detail::make_caster<U>();
        if(!caster.load(source, convert)) {
          return;
        }
        m_value.emplace(pybind11::detail::cast_op<U&&>(std::move(caster)));
        is_converted = true;
      });
    return is_converted;
  }
}

#endif
