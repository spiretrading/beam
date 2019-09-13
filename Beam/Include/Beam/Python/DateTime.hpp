#ifndef BEAM_PYTHON_DATE_TIME_HPP
#define BEAM_PYTHON_DATE_TIME_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace pybind11::detail {
  template<>
  struct BEAM_EXPORT_DLL type_caster<boost::posix_time::time_duration>
      : Beam::Python::BasicTypeCaster<boost::posix_time::time_duration> {
    static handle cast(boost::posix_time::time_duration value,
      return_value_policy policy, handle parent);
    bool load(handle source, bool);
  };

  template<>
  struct BEAM_EXPORT_DLL type_caster<boost::posix_time::ptime>
      : Beam::Python::BasicTypeCaster<boost::posix_time::ptime> {
    static handle cast(boost::posix_time::ptime value,
      return_value_policy policy, handle parent);
    bool load(handle source, bool);
  };

  extern template struct type_caster<boost::posix_time::time_duration>;
  extern template struct type_caster<boost::posix_time::ptime>;
}

#endif
