#ifndef BEAM_TOLOCALTIME_HPP
#define BEAM_TOLOCALTIME_HPP
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/local_time/local_time_types.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace Beam {

  /**
   * Converts a timestamp from UTC to the local time.
   * @param time The UTC time to convert.
   * @return The UTC time converted to local time.
   */
  inline boost::posix_time::ptime to_local_time(boost::posix_time::ptime time) {
    if(time.is_special()) {
      return time;
    }
    try {
      return boost::date_time::c_local_adjustor<
        boost::posix_time::ptime>::utc_to_local(time);
    } catch(const std::exception&) {
      return boost::posix_time::not_a_date_time;
    }
  }

  /**
   * Converts a timestamp from local time to UTC.
   * @param time The local time to convert.
   * @return The local time converted to UTC time.
   */
  inline boost::posix_time::ptime to_utc_time(boost::posix_time::ptime time) {
    if(time.is_special()) {
      return time;
    }
    try {
      return time - (boost::date_time::c_local_adjustor<
        boost::posix_time::ptime>::utc_to_local(time) - time);
    } catch(const std::exception&) {
      return boost::posix_time::not_a_date_time;
    }
  }

  /**
   * Converts a time of day for the current date from UTC to the local time.
   * @param time The UTC time of day to convert.
   * @return The timestamp for the original time of day in local time.
   */
  inline boost::posix_time::time_duration to_local_time(
      boost::posix_time::time_duration time) {
    if(time.is_special()) {
      return time;
    }
    auto current_time = boost::posix_time::second_clock::local_time();
    auto delta = to_utc_time(current_time) - current_time;
    auto local_time = time - delta;
    if(local_time.hours() < 0) {
      local_time += boost::posix_time::hours(24);
    }
    return local_time;
  }

  /**
   * Converts a time_duration from local time to UTC.
   * @param time The local time to convert.
   * @return The local time converted to UTC time.
   */
  inline boost::posix_time::time_duration to_utc_time(
      boost::posix_time::time_duration time) {
    if(time.is_special()) {
      return time;
    }
    auto current_time = boost::posix_time::second_clock::local_time();
    auto delta = to_utc_time(current_time) - current_time;
    auto utc_time = time + delta;
    if(utc_time.hours() >= 24) {
      utc_time -= boost::posix_time::hours(24);
    }
    return utc_time;
  }

  /**
   * Adjusts a date/time from one time zone to another.
   * @param time The time to adjust.
   * @param source The time zone the <i>time</i> is in.
   * @param target The time zone to convert the <i>time</i> to.
   * @param time_zone_database The time zone database used.
   * @return The result of converting <i>time</i> from time zone <i>source</i>
   *         to time zone <i>target</i>.
   */
  inline boost::posix_time::ptime adjust_date_time(
      boost::posix_time::ptime time, const std::string& source,
      const std::string& target,
      const boost::local_time::tz_database& time_zone_database) {
    if(time.is_special()) {
      return time;
    }
    auto source_time_zone = time_zone_database.time_zone_from_region(source);
    if(!source_time_zone) {
      return boost::posix_time::not_a_date_time;
    }
    auto target_time_zone = time_zone_database.time_zone_from_region(target);
    if(!target_time_zone) {
      return boost::posix_time::not_a_date_time;
    }
    auto source_date_time = boost::local_time::local_date_time(
      time.date(), time.time_of_day(), source_time_zone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    if(source_date_time.is_not_a_date_time()) {
      return boost::posix_time::not_a_date_time;
    }
    auto target_date_time = source_date_time.local_time_in(target_time_zone);
    if(target_date_time.is_not_a_date_time()) {
      return boost::posix_time::not_a_date_time;
    }
    auto result = boost::posix_time::ptime(target_date_time.local_time().date(),
      target_date_time.local_time().time_of_day());
    return result;
  }
}

#endif
