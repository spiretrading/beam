#ifndef BEAM_PYTHON_BASIC_TYPE_CASTER_HPP
#define BEAM_PYTHON_BASIC_TYPE_CASTER_HPP
#include <optional>
#include <type_traits>
#include <pybind11/pybind11.h>

namespace pybind11::detail {

  /**
   * Implements the common functionality needed to perform type casting of
   * Python objects via pybind11.
   * @param <T> The C++ type to cast to-from Python.
   */
  template<typename T>
  struct BasicTypeCaster {
    inline static std::optional<T> m_value;

    static PYBIND11_DESCR name() {
      return type_descr(_(typeid(T).name()));
    }

    template<typename U, typename = std::enable_if_t<
      std::is_same_v<T, std::remove_cv_t<U>>>>
    static handle cast(U* source, return_value_policy policy, handle parent) {
      if(source == nullptr) {
        return none().release();
      } else if(policy == return_value_policy::take_ownership) {
        auto h = cast(std::move(*source), policy, parent);
        delete source;
        return h;
      } else {
        return cast(*source, policy, parent);
      }
    }

    operator T* () {
      return &*m_value;
    }

    operator T& () {
      return *m_value;
    }

    operator T&& () && {
      return std::move(*m_value);
    }

    template<typename U>
    using cast_op_type = pybind11::detail::movable_cast_op_type<U>;
  };
}

#endif
