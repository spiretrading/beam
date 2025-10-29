#ifndef BEAM_PYTHON_DATE_TIME_HPP
#define BEAM_PYTHON_DATE_TIME_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <datetime.h>
#include "Beam/Python/BasicTypeCaster.hpp"
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
      auto total_microseconds = std::abs(value.total_microseconds());
      auto total_seconds = total_microseconds / 1000000;
      auto days = total_seconds / 86400;
      auto seconds = total_seconds - (86400 * days);
      auto microseconds = total_microseconds - 1000000 * total_seconds;
      if(days != 0) {
        if(total_microseconds < 0) {
          days = -days;
        }
      } else if(seconds != 0) {
        if(total_microseconds < 0) {
          seconds = -seconds;
        }
      } else {
        if(total_microseconds < 0) {
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
      auto is_negative = (days < 0);
      if(is_negative) {
        days = -days;
      }
      m_value.emplace(boost::posix_time::hours(24) * days +
        boost::posix_time::seconds(delta->seconds) +
        boost::posix_time::microseconds(delta->microseconds));
      if(is_negative) {
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
      auto time_of_day = value.time_of_day();
      auto usec = static_cast<int>(time_of_day.total_microseconds() % 1000000);
      return PyDateTime_FromDateAndTime(day.year(), day.month(), day.day(),
        static_cast<int>(time_of_day.hours()),
        static_cast<int>(time_of_day.minutes()),
        static_cast<int>(time_of_day.seconds()), usec);
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
      auto time_of_day = boost::posix_time::time_duration(
        PyDateTime_DATE_GET_HOUR(source.ptr()),
        PyDateTime_DATE_GET_MINUTE(source.ptr()),
        PyDateTime_DATE_GET_SECOND(source.ptr()));
      auto us = boost::posix_time::microsec(
        PyDateTime_DATE_GET_MICROSECOND(source.ptr()));
      time_of_day += us;
      if(d.year() == 9999 && d.month() == 12 && d.day() == 31 &&
          time_of_day.hours() == 23 && time_of_day.minutes() == 59 &&
          time_of_day.seconds() == 59 &&
          us == boost::posix_time::microsec(999999)) {
        m_value.emplace(boost::posix_time::pos_infin);
      } else if(d.year() == 1 && d.month() == 1 && d.day() == 1 &&
          time_of_day.hours() == 0 && time_of_day.minutes() == 0 &&
          time_of_day.seconds() == 0 &&
          us == boost::posix_time::microsec(0)) {
        m_value.emplace(boost::posix_time::neg_infin);
      } else {
        m_value.emplace(d, time_of_day);
      }
      return !PyErr_Occurred();
    }
  };
}

#endif
