#ifndef BEAM_PYTHON_DATE_TIME_HPP
#define BEAM_PYTHON_DATE_TIME_HPP
#include <concepts>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <datetime.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace pybind11::detail {
  template<typename T> requires
    std::same_as<T, boost::gregorian::greg_day> ||
    std::same_as<T, boost::gregorian::greg_month> ||
    std::same_as<T, boost::gregorian::greg_weekday> ||
    std::same_as<T, boost::gregorian::greg_year>
  struct type_caster<T> : Beam::Python::BasicTypeCaster<T> {
    static constexpr auto name = pybind11::detail::_("int");
    static handle cast(T value, return_value_policy policy, handle parent) {
      return type_caster<unsigned short>::cast(
        static_cast<unsigned short>(value), policy, parent);
    }

    bool load(handle source, bool convert) {
      auto value = type_caster<unsigned short>();
      if(!value.load(source, convert)) {
        return false;
      }
      this->m_value.emplace(cast_op<unsigned short>(value));
      return true;
    }
  };

  template<>
  struct type_caster<boost::gregorian::date>
      : Beam::Python::BasicTypeCaster<boost::gregorian::date> {
    static constexpr auto name = pybind11::detail::_("datetime.date");
    static handle cast(boost::gregorian::date value,
        return_value_policy policy, handle parent) {
      if(!PyDateTimeAPI) {
        PyDateTime_IMPORT;
      }
      if(value.is_not_a_date()) {
        return none().release();
      } else if(value.is_neg_infinity()) {
        return PyDate_FromDate(1, 1, 1);
      } else if(value.is_pos_infinity()) {
        return PyDate_FromDate(9999, 12, 31);
      }
      return PyDate_FromDate(value.year(), value.month(), value.day());
    }

    bool load(handle source, bool) {
      if(!PyDateTimeAPI) {
        PyDateTime_IMPORT;
      }
      if(source.is_none()) {
        m_value.emplace(boost::date_time::not_a_date_time);
        return true;
      }
      if(!PyDate_Check(source.ptr())) {
        return false;
      }
      auto year = PyDateTime_GET_YEAR(source.ptr());
      auto month = PyDateTime_GET_MONTH(source.ptr());
      auto day = PyDateTime_GET_DAY(source.ptr());
      if(year == 1 && month == 1 && day == 1) {
        m_value.emplace(boost::date_time::neg_infin);
      } else if(year == 9999 && month == 12 && day == 31) {
        m_value.emplace(boost::date_time::pos_infin);
      } else {
        m_value.emplace(year, month, day);
      }
      return true;
    }
  };

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
