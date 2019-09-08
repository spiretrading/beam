#ifndef BEAM_PYTHON_VARIANT_HPP
#define BEAM_PYTHON_VARIANT_HPP
#include <boost/variant/variant.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pybind11::detail {
  template<typename... T>
  struct type_caster<boost::variant<T...>> :
    variant_caster<boost::variant<T...>> {};

  template<>
  struct visit_helper<boost::variant> {
    template<typename... A>
    static auto call(A&&... args) -> decltype(boost::apply_visitor(args...)) {
      return boost::apply_visitor(args...);
    }
  };
}

#endif
