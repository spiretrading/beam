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
namespace TimeService {

  //! Returns a time zone's the offset, including DST, from UTC at a given time.
  /*!
    \param currentTime The current time to get the UTC offset of.
    \param timeZone The <i>currentTime</i>'s time zone.
    \return The <i>currentTime</i>'s offset, including DST, from UTC.
  */
  inline boost::posix_time::time_duration GetUtcOffset(
      const boost::posix_time::ptime& currentTime,
      const boost::local_time::time_zone& timeZone) {
    auto baseOffset = timeZone.base_utc_offset();
    auto dstOffset = timeZone.dst_offset();
    auto dstStart = timeZone.dst_local_start_time(currentTime.date().year()) -
      baseOffset;
    auto dstEnd = timeZone.dst_local_end_time(currentTime.date().year()) -
      baseOffset - dstOffset;
    auto offset = baseOffset;
    if(currentTime >= dstStart && currentTime <= dstEnd) {
      offset += dstOffset;
    }
    return offset;
  }

  //! Converts a ptime from UTC to the local time.
  /*!
    \param time The UTC time to convert.
    \return The UTC time converted to local time.
  */
  inline boost::posix_time::ptime ToLocalTime(
      const boost::posix_time::ptime& time) {
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

  //! Converts a ptime from local time to UTC.
  /*!
    \param time The local time to convert.
    \return The local time converted to UTC time.
  */
  inline boost::posix_time::ptime ToUtcTime(
      const boost::posix_time::ptime& time) {
    if(time.is_special()) {
      return time;
    }
    try {
      auto t11 = boost::date_time::c_local_adjustor<
        boost::posix_time::ptime>::utc_to_local(time);
      auto td = t11 - time;
      return time - td;
    } catch(const std::exception&) {
      return boost::posix_time::not_a_date_time;
    }
  }

  //! Converts a time_duration from UTC to the local time.
  /*!
    \param time The UTC time to convert.
    \return The UTC time converted to local time.
  */
  inline boost::posix_time::time_duration ToLocalTime(
      const boost::posix_time::time_duration& time) {
    if(time.is_special() || time.is_not_a_date_time()) {
      return time;
    }
    auto currentTime = boost::posix_time::second_clock::local_time();
    auto delta = ToUtcTime(currentTime) - currentTime;
    auto localTime = time - delta;
    if(localTime.hours() < 0) {
      localTime += boost::posix_time::hours(24);
    }
    return localTime;
  }

  //! Converts a time_duration from local time to UTC.
  /*!
    \param time The local time to convert.
    \return The local time converted to UTC time.
  */
  inline boost::posix_time::time_duration ToUtcTime(
      const boost::posix_time::time_duration& time) {
    if(time.is_special() || time.is_not_a_date_time()) {
      return time;
    }
    auto currentTime = boost::posix_time::second_clock::local_time();
    auto delta = ToUtcTime(currentTime) - currentTime;
    auto utcTime = time + delta;
    if(utcTime.hours() >= 24) {
      utcTime -= boost::posix_time::hours(24);
    }
    return utcTime;
  }

  //! Adjusts a date/time from one time zone to another.
  /*!
    \param time The time to adjust.
    \param source The time zone the <i>time</i> is in.
    \param target The time zone to convert the <i>time</i> to.
    \param timeZoneDatabase The time zone database used.
    \return The result of converting <i>time</i> from time zone <i>source</i>
            to time zone <i>target</i>.
  */
  inline boost::posix_time::ptime AdjustDateTime(
      const boost::posix_time::ptime& time, const std::string& source,
      const std::string& target,
      const boost::local_time::tz_database& timeZoneDatabase) {
    boost::local_time::time_zone_ptr sourceTimeZone =
      timeZoneDatabase.time_zone_from_region(source);
    boost::local_time::time_zone_ptr targetTimeZone =
      timeZoneDatabase.time_zone_from_region(target);
    boost::local_time::local_date_time sourceDateTime(time.date(),
      time.time_of_day(), sourceTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    boost::local_time::local_date_time targetDateTime =
      sourceDateTime.local_time_in(targetTimeZone);
    boost::posix_time::ptime result(targetDateTime.local_time().date(),
      targetDateTime.local_time().time_of_day());
    return result;
  }
}
}

#endif
