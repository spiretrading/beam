#ifndef BEAM_PYTHON_DECIMAL_HPP
#define BEAM_PYTHON_DECIMAL_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {
namespace Details {
  inline auto PyDecimal() {
    static auto decimal = pybind11::module::import("decimal").attr("Decimal");
    return decimal;
  }
}

  /**
   * Implements a type caster for Python's Decimal.
   * @param <T> The C++ type to convert a Python Decimal to.
   */
  template<typename T>
  struct DecimalTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static constexpr auto name = pybind11::detail::_("Decimal");
    static pybind11::handle cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle DecimalTypeCaster<T>::cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return Details::PyDecimal()(static_cast<std::string>(value)).release();
  }

  template<typename T>
  bool DecimalTypeCaster<T>::load(pybind11::handle source, bool) {
    if(!PyObject_IsInstance(source.ptr(), Details::PyDecimal().ptr())) {
      return false;
    }
    auto str = PyObject_Str(source.ptr());
    auto value = PyUnicode_AsUTF8(str);
    if(PyErr_Occurred()) {
      return false;
    }
    m_value.emplace(value);
    return true;
  }
}

namespace pybind11::detail {
  template<unsigned D, typename E, typename A>
  struct type_caster<boost::multiprecision::cpp_dec_float<D, E, A>> :
    Beam::Python::DecimalTypeCaster<
    boost::multiprecision::cpp_dec_float<D, E, A>> {};
}

#endif
