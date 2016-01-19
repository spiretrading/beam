#ifndef BEAM_POSIXTIMETOMYSQLDATETIME_HPP
#define BEAM_POSIXTIMETOMYSQLDATETIME_HPP
#include <ctime>
#include <cstdint>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/MySql/MySql.hpp"
#include <mysql++/mysql++.h>

namespace Beam {
namespace MySql {

  //! Converts a POSIX timestamp into an int for storage in MySQL.
  /*!
    \param timestamp The POSIX timestamp to convert.
    \return An integer that can be used for storage of a timestamp in MySQL.
  */
  inline std::uint64_t ToMySqlTimestamp(
      const boost::posix_time::ptime& timestamp) {
    static const boost::posix_time::ptime BASE(
      boost::gregorian::date(1970, boost::gregorian::Jan, 1),
      boost::posix_time::seconds(0));
    if(timestamp == boost::posix_time::not_a_date_time) {
      return std::numeric_limits<std::uint64_t>::max();
    } else if(timestamp == boost::posix_time::pos_infin) {
      return std::numeric_limits<std::uint64_t>::max() - 1;
    } else if(timestamp == boost::posix_time::neg_infin) {
      return 0;
    }
    boost::posix_time::time_duration delta = timestamp - BASE;
    return delta.total_milliseconds();
  }

  //! Converts a timestamp in MySQL into a POSIX timestamp.
  /*!
    \param timestamp The MySQL timestamp to convert.
    \return The POSIX timestamp.
  */
  inline boost::posix_time::ptime FromMySqlTimestamp(std::uint64_t timestamp) {
    static const boost::posix_time::ptime BASE(
      boost::gregorian::date(1970, boost::gregorian::Jan, 1),
      boost::posix_time::seconds(0));
    if(timestamp == std::numeric_limits<std::uint64_t>::max()) {
      return boost::posix_time::not_a_date_time;
    } else if(timestamp == std::numeric_limits<std::uint64_t>::max() - 1) {
      return boost::posix_time::pos_infin;
    } else if(timestamp == 0) {
      return boost::posix_time::neg_infin;
    }
    boost::posix_time::ptime result = BASE +
      boost::posix_time::milliseconds(timestamp);
    return result;
  }

  //! Converts a MySQL DateTime to a boost::posix_time::ptime.
  inline boost::posix_time::ptime FromDateTime(
      const mysqlpp::DateTime& source) {
    if(source == mysqlpp::DateTime(3999, 1, 1, 1, 1, 1)) {
      return boost::posix_time::pos_infin;
    } else if(source == mysqlpp::DateTime(std::time_t(0))) {
      return boost::posix_time::not_a_date_time;
    }
    std::tm pt_tm;
    pt_tm.tm_year = source.year() - 1900;
    pt_tm.tm_mon = source.month() - 1;
    pt_tm.tm_mday = source.day();
    pt_tm.tm_hour = source.hour();
    pt_tm.tm_min = source.minute();
    pt_tm.tm_sec = source.second();
    return boost::posix_time::ptime_from_tm(pt_tm);
  }

  //! Converts a boost::posix_time::ptime to a MySQL DateTime.
  inline mysqlpp::DateTime ToDateTime(const boost::posix_time::ptime& source) {
    if(source == boost::posix_time::pos_infin) {
      return mysqlpp::DateTime(3999, 1, 1, 1, 1, 1);
    } else if(source.is_special()) {
      return mysqlpp::DateTime(std::time_t(0));
    } else {
      return mysqlpp::DateTime(
        static_cast<unsigned short>(source.date().year()),
        static_cast<unsigned char>(source.date().month()),
        static_cast<unsigned char>(source.date().day()),
        static_cast<unsigned char>(source.time_of_day().hours()),
        static_cast<unsigned char>(source.time_of_day().minutes()),
        static_cast<unsigned char>(source.time_of_day().seconds()));
    }
  }
}
}

#endif
