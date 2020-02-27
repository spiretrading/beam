#ifndef BEAM_PYTHON_RATIONAL_HPP
#define BEAM_PYTHON_RATIONAL_HPP
#include <boost/rational.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {
namespace Details {
  inline auto PyFraction() {
    static auto fraction = pybind11::module::import(
      "fractions").attr("Fraction");
    return fraction;
  }
}

  template<typename T>
  struct RationalTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static constexpr auto name = pybind11::detail::_("Rational");
    static pybind11::handle cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle RationalTypeCaster<T>::cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return Details::PyFraction()(value.numerator(),
      value.denominator()).release();
  }

  template<typename T>
  bool RationalTypeCaster<T>::load(pybind11::handle source, bool) {
    if(!PyObject_IsInstance(source.ptr(), Details::PyFraction().ptr())) {
      return false;
    }
    m_value.emplace(source.attr("numerator").cast<typename Type::int_type>(),
      source.attr("denominator").cast<typename Type::int_type>());
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<boost::rational<T>> : Beam::Python::RationalTypeCaster<
    boost::rational<T>> {};
}

#endif
