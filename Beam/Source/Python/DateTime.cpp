#include "Beam/Python/DateTime.hpp"
#include <datetime.h>

using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;
using namespace pybind11::detail;

namespace {
  std::optional<boost::posix_time::time_duration> m_value;
}

PYBIND11_DESCR type_caster<time_duration>::name() {
  return type_descr(_("TimeDuration"));
}

handle type_caster<time_duration>::cast(time_duration source,
    return_value_policy policy, handle parent) {
  auto totalMicroseconds = std::abs(source.total_microseconds());
  auto totalSeconds = totalMicroseconds / 1000000;
  auto days = totalSeconds / 86400;
  auto seconds = totalSeconds - (86400 * days);
  auto microseconds = totalMicroseconds - 1000000 * totalSeconds;
  if(days != 0) {
    if(totalMicroseconds < 0) {
      days = -days;
    }
  } else if(seconds != 0) {
    if(totalMicroseconds < 0) {
      seconds = -seconds;
    }
  } else {
    if(totalMicroseconds < 0) {
      microseconds = -microseconds;
    }
  }
  return PyDelta_FromDSU(static_cast<int>(days), static_cast<int>(seconds),
    static_cast<int>(microseconds));
}

type_caster<time_duration>::operator time_duration* () {
  return &*m_value;
}

type_caster<time_duration>::operator time_duration& () {
  return *m_value;
}

type_caster<time_duration>::operator time_duration&& () && {
  return std::move(*m_value);
}

bool type_caster<time_duration>::load(handle source, bool) {
  if(!PyDateTimeAPI) {
    PyDateTime_IMPORT;
  }
  if(!PyDelta_Check(source.ptr())) {
    return false;
  }
  auto delta = reinterpret_cast<PyDateTime_Delta*>(source.ptr());
  auto days = delta->days;
  auto isNegative = (days < 0);
  if(isNegative) {
    days = -days;
  }
  m_value.emplace(hours(24) * days + seconds(delta->seconds) +
    microseconds(delta->microseconds));
  if(isNegative) {
    m_value = m_value->invert_sign();
  }
  return !PyErr_Occurred();
}
