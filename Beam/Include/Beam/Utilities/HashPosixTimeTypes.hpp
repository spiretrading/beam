#ifndef BEAM_HASH_PTIME_HPP
#define BEAM_HASH_PTIME_HPP
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace boost::posix_time {
  inline std::size_t hash_value(ptime value) {
    static const auto EPOCH =
      ptime(boost::gregorian::date(1970, 1, 1), time_duration(0, 0, 0));
    auto ticks = (value - EPOCH).ticks();
    return std::hash<decltype(ticks)>()(ticks);
  }

  inline std::size_t hash_value(time_duration value) {
    return std::hash<decltype(value.ticks())>()(value.ticks());
  }
}

namespace std {
  template<>
  struct hash<boost::posix_time::ptime> {
    std::size_t
        operator ()(const boost::posix_time::ptime& value) const noexcept {
      return boost::posix_time::hash_value(value);
    }
  };

  template<>
  struct hash<boost::posix_time::time_duration> {
    std::size_t operator ()(
        const boost::posix_time::time_duration& value) const noexcept {
      return boost::posix_time::hash_value(value);
    }
  };
}

#endif
