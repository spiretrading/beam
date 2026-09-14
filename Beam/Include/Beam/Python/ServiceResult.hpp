#ifndef BEAM_PYTHON_SERVICE_RESULT_HPP
#define BEAM_PYTHON_SERVICE_RESULT_HPP
#include <concepts>
#include <string_view>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonException.hpp"
#include "Beam/ServicesTests/ServiceResult.hpp"

namespace Beam::Python {

  /**
   * Exports a test service result.
   * @tparam T The type of result returned to the caller.
   * @param module The module to export to.
   * @param name The name of the class.
   */
  template<typename T>
  void export_service_result(pybind11::module& module, std::string_view name) {
    using Result = Tests::ServiceResult<T>;
    auto result = pybind11::class_<Result>(module, name.data());
    if constexpr(std::same_as<T, void>) {
      result.def("set", pybind11::overload_cast<>(&Result::set),
        pybind11::call_guard<GilRelease>());
    } else {
      result.def("set", pybind11::overload_cast<const T&>(&Result::set),
        pybind11::call_guard<GilRelease>());
    }
    result.def("set_exception", [] (Result& self,
        const pybind11::object& exception) {
      if(!PyExceptionInstance_Check(exception.ptr())) {
        throw pybind11::type_error(
          "set_exception() requires an exception instance.");
      }
      auto error = std::make_exception_ptr(PythonException(exception));
      auto release = GilRelease();
      self.set(error);
    });
  }
}

#endif
