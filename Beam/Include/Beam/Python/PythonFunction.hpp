#ifndef BEAM_PYTHON_PYTHON_FUNCTION_HPP
#define BEAM_PYTHON_PYTHON_FUNCTION_HPP
#include <utility>
#include <pybind11/pybind11.h>
#include "Beam/Python/SharedObject.hpp"

namespace Beam::Python {
  template<typename>
  class PythonFunction;

  /**
   * A function wrapper that stores a Python callable in a thread-safe manner.
   * @tparam R The return type of the function.
   * @tparam Args The argument types of the function.
   */
  template<typename R, typename... Args>
  class PythonFunction<R (Args...)> {
    public:

      /** Constructs an empty PythonFunction. */
      PythonFunction() = default;

      /**
       * Constructs a PythonFunction from a Python callable.
       * @param callable The Python object to wrap.
       */
      explicit PythonFunction(pybind11::object callable);

      /** Tests if this PythonFunction is empty. */
      explicit operator bool() const;

      /**
       * Invokes the underlying Python callable.
       * @param args The arguments to pass to the callable.
       * @return The result of the Python callable.
       */
      R operator ()(Args... args) const;

    private:
      SharedObject m_callable;
  };

  template<typename R, typename... Args>
  PythonFunction<R (Args...)>::PythonFunction(pybind11::object callable)
    : m_callable(std::move(callable)) {}

  template<typename R, typename... Args>
  PythonFunction<R (Args...)>::operator bool() const {
    auto lock = pybind11::gil_scoped_acquire();
    return !m_callable->is_none();
  }

  template<typename R, typename... Args>
  R PythonFunction<R (Args...)>::operator ()(Args... args) const {
    auto lock = pybind11::gil_scoped_acquire();
    if constexpr(std::is_void_v<R>) {
      (*m_callable)(std::forward<Args>(args)...);
    } else {
      return (*m_callable)(std::forward<Args>(args)...).template cast<R>();
    }
  }

  /**
   * Implements a type caster for PythonFunction.
   * @tparam R The return type of the function.
   * @tparam Args The argument types of the function.
   */
  template<typename R, typename... Args>
  struct PythonFunctionTypeCaster {
    using Type = PythonFunction<R (Args...)>;
    static constexpr auto name = pybind11::detail::_("Callable[[") +
      pybind11::detail::concat(pybind11::detail::make_caster<Args>::name...) +
      pybind11::detail::_("], ") + pybind11::detail::make_caster<R>::name +
      pybind11::detail::_("]");

    bool load(pybind11::handle source, bool convert) {
      if(!source || source.is_none()) {
        return false;
      }
      if(!PyCallable_Check(source.ptr())) {
        return false;
      }
      m_value.emplace(pybind11::reinterpret_borrow<pybind11::object>(source));
      return true;
    }

    static pybind11::handle cast(const Type& source,
        pybind11::return_value_policy policy, pybind11::handle parent) {
      if(!source) {
        return pybind11::none().release();
      }
      auto lock = pybind11::gil_scoped_acquire();
      return (*source.m_callable).inc_ref();
    }

    template<typename T>
    using cast_op_type = pybind11::detail::cast_op_type<T>;

    operator Type*() {
      return &*m_value;
    }

    operator Type&() {
      return *m_value;
    }

    operator Type&&() && {
      return std::move(*m_value);
    }

    std::optional<Type> m_value;
  };
}

namespace pybind11::detail {
  template<typename R, typename... Args>
  struct type_caster<Beam::Python::PythonFunction<R (Args...)>> :
    Beam::Python::PythonFunctionTypeCaster<R, Args...> {};
}

#endif
