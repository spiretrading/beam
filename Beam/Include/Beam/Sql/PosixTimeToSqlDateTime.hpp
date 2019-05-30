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
    static const auto BASE = boost::posix_time::ptime(
      boost::gregorian::date(1970, boost::gregorian::Jan, 1),
      boost::posix_time::seconds(0));
    if(timestamp == boost::posix_time::not_a_date_time) {
      return std::numeric_limits<std::uint64_t>::max();
    } else if(timestamp == boost::posix_time::pos_infin) {
      return std::numeric_limits<std::uint64_t>::max() - 1;
    } else if(timestamp == boost::posix_time::neg_infin) {
      return 0;
    }
    auto delta = timestamp - BASE;
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
    auto result = BASE + boost::posix_time::milliseconds(timestamp);
    return result;
  }
}

namespace Viper {
  template<>
  inline const auto native_to_data_type_v<boost::posix_time::ptime> = big_uint;

  template<>
  struct ToSql<boost::posix_time::ptime> {
    void operator ()(boost::posix_time::ptime value,
        std::string& column) const {
      return to_sql(Beam::ToSqlTimestamp(value), column);
    }
  };

  template<>
  struct FromSql<boost::posix_time::ptime> {
    auto operator ()(const RawColumn& column) const {
      return Beam::FromSqlTimestamp(from_sql<std::uint64_t>(column));
    }
  };
}

#endif
