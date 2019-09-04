#ifndef BEAM_PYTHON_DATE_TIME_HPP
#define BEAM_PYTHON_DATE_TIME_HPP
#include <optional>
#include <type_traits>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Utilities/DllExport.hpp"

namespace pybind11::detail {
  template<>
  struct BEAM_EXPORT_DLL type_caster<boost::posix_time::time_duration> {
    static PYBIND11_DESCR name();

    template<typename T, typename = std::enable_if_t<
      std::is_same_v<boost::posix_time::time_duration, std::remove_cv_t<T>>>>
    static handle cast(T* source, return_value_policy policy, handle parent) {
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

    static handle cast(boost::posix_time::time_duration source,
      return_value_policy policy, handle parent);

    operator boost::posix_time::time_duration* ();
    operator boost::posix_time::time_duration& ();
    operator boost::posix_time::time_duration&& () &&;

    template<typename T>
    using cast_op_type = pybind11::detail::movable_cast_op_type<T>;

    bool load(handle source, bool);
  };
}

#endif
