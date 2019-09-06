#ifndef BEAM_PYTHON_OPTIONAL_HPP
#define BEAM_PYTHON_OPTIONAL_HPP
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace pybind11::detail {
  template <typename T>
  struct type_caster<boost::optional<T>> :
    optional_caster<boost::optional<T>> {};
}

#endif
