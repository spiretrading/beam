#ifndef BEAM_PYTHON_DATE_TIME_HPP
#define BEAM_PYTHON_DATE_TIME_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Python/BasicTypeCaster.hpp"
#include <datetime.h>
#include "Beam/Utilities/DllExport.hpp"

namespace pybind11::detail {
  template<>
  struct type_caster<boost::posix_time::time_duration>
      : Beam::Python::BasicTypeCaster<boost::posix_time::time_duration> {
    static constexpr auto name = pybind11::detail::_("TimeDuration");
    static handle cast(boost::posix_time::time_duration value,
        return_value_policy policy, handle parent) {
      if(!PyDateTimeAPI) {
        PyDateTime_IMPORT;
      }
      auto totalMicroseconds = std::abs(value.total_microseconds());
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

    bool load(handle source, bool) {
      if(!PyDateTimeAPI) {
        PyDateTime_IMPORT;
      }
      if(source.is_none()) {
        m_value.emplace(boost::posix_time::not_a_date_time);
        return true;
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
      m_value.emplace(boost::posix_time::hours(24) * days +
        boost::posix_time::seconds(delta->seconds) +
        boost::posix_time::microseconds(delta->microseconds));
      if(isNegative) {
        m_value = m_value->invert_sign();
      }
      return !PyErr_Occurred();
    }
  };

  template<>
  struct type_caster<boost::posix_time::ptime>
      : Beam::Python::BasicTypeCaster<boost::posix_time::ptime> {
    static constexpr auto name = pybind11::detail::_("DateTime");
    static handle cast(boost::posix_time::ptime value,
        return_value_policy policy, handle parent) {
      if(!PyDateTimeAPI) {
        PyDateTime_IMPORT;
      }
      if(value == boost::posix_time::not_a_date_time) {
        return Py_None;
      } else if(value == boost::posix_time::neg_infin) {
        return PyDateTime_FromDateAndTime(1, 1, 1, 0, 0, 0, 0);
      } else if(value == boost::posix_time::pos_infin) {
        return PyDateTime_FromDateAndTime(9999, 12, 31, 23, 59, 59, 999999);
      }
      auto day = value.date();
      auto timeOfDay = value.time_of_day();
      auto usec = static_cast<int>(timeOfDay.total_microseconds() % 1000000);
      return PyDateTime_FromDateAndTime(day.year(), day.month(), day.day(),
        static_cast<int>(timeOfDay.hours()),
        static_cast<int>(timeOfDay.minutes()),
        static_cast<int>(timeOfDay.seconds()), usec);
    }

    bool load(handle source, bool) {
      if(!PyDateTimeAPI) {
        PyDateTime_IMPORT;
      }
      if(source.is_none()) {
        m_value.emplace(boost::posix_time::not_a_date_time);
        return true;
      }
      if(!PyDateTime_Check(source.ptr())) {
        return false;
      }
      auto d = boost::gregorian::date(PyDateTime_GET_YEAR(source.ptr()),
        PyDateTime_GET_MONTH(source.ptr()), PyDateTime_GET_DAY(source.ptr()));
      auto timeOfDay = boost::posix_time::time_duration(
        PyDateTime_DATE_GET_HOUR(source.ptr()),
        PyDateTime_DATE_GET_MINUTE(source.ptr()),
        PyDateTime_DATE_GET_SECOND(source.ptr()));
      auto us = boost::posix_time::microsec(
        PyDateTime_DATE_GET_MICROSECOND(source.ptr()));
      timeOfDay += us;
      if(d.year() == 9999 && d.month() == 12 && d.day() == 31 &&
          timeOfDay.hours() == 23 && timeOfDay.minutes() == 59 &&
          timeOfDay.seconds() == 59 &&
          us == boost::posix_time::microsec(999999)) {
        m_value.emplace(boost::posix_time::pos_infin);
      } else if(d.year() == 1 && d.month() == 1 && d.day() == 1 &&
          timeOfDay.hours() == 0 && timeOfDay.minutes() == 0 &&
          timeOfDay.seconds() == 0 && us == boost::posix_time::microsec(0)) {
        m_value.emplace(boost::posix_time::neg_infin);
      } else {
        m_value.emplace(d, timeOfDay);
      }
      return !PyErr_Occurred();
    }
  };
}

#endif
