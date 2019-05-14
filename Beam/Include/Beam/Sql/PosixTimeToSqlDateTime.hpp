#ifndef BEAM_POSIX_TIME_TO_SQL_DATE_TIME_HPP
#define BEAM_POSIX_TIME_TO_SQL_DATE_TIME_HPP
#include <ctime>
#include <cstdint>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <Viper/Viper.hpp>

namespace Beam {

  //! Converts a POSIX timestamp into an int for storage in an SQL row.
  /*!
    \param timestamp The POSIX timestamp to convert.
    \return An integer that can be used for storage of a timestamp in an
            SQL row.
  */
  inline std::uint64_t ToSqlTimestamp(
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

  //! Converts a timestamp in an SQL row into a POSIX timestamp.
  /*!
    \param timestamp The SQL timestamp to convert.
    \return The POSIX timestamp.
  */
  inline boost::posix_time::ptime FromSqlTimestamp(std::uint64_t timestamp) {
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
}

namespace Viper {
  template<>
  inline const auto native_to_data_type_v<boost::posix_time::ptime> =
    DateTimeDataType();

  template<>
  struct ToSql<boost::posix_time::ptime> {
    void operator ()(boost::posix_time::ptime value,
        std::string& column) const {
      if(value == boost::posix_time::pos_infin) {
        to_sql(DateTime(3999, 1, 1, 1, 1, 1, 0), column);
      } else if(value.is_special()) {
        to_sql(DateTime(0, 0, 0, 0, 0, 0, 0), column);
      } else {
        to_sql(DateTime(value.date().year(), value.date().month(),
          value.date().day(), static_cast<int>(value.time_of_day().hours()),
          static_cast<int>(value.time_of_day().minutes()),
          static_cast<int>(value.time_of_day().seconds()), 0), column);
      }
    }
  };

  template<>
  struct FromSql<boost::posix_time::ptime> {
    boost::posix_time::ptime operator ()(const RawColumn& column) const {
      auto dateTime = from_sql<DateTime>(column);
      if(dateTime == DateTime(3999, 1, 1, 1, 1, 1, 0)) {
        return boost::posix_time::pos_infin;
      } else if(dateTime == DateTime(0, 0, 0, 0, 0, 0, 0)) {
        return boost::posix_time::not_a_date_time;
      }
      return boost::posix_time::ptime_from_tm(to_tm(dateTime));
    }
  };
}

#endif
