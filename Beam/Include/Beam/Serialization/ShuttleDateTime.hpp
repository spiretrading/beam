#ifndef BEAM_SHUTTLE_DATE_TIME_HPP
#define BEAM_SHUTTLE_DATE_TIME_HPP
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam::Serialization {
  template<>
  struct IsStructure<boost::posix_time::time_duration> : std::false_type {};

  template<>
  struct Send<boost::posix_time::time_duration> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::posix_time::time_duration& value) const {
      shuttle.Shuttle(name, boost::posix_time::to_simple_string(value));
    }
  };

  template<>
  struct Receive<boost::posix_time::time_duration> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::posix_time::time_duration& value) const {
      auto timeAsString = std::string();
      shuttle.Shuttle(name, timeAsString);
      if(timeAsString == "+infinity") {
        value = boost::posix_time::pos_infin;
      } else if(timeAsString == "-infinity") {
        value = boost::posix_time::neg_infin;
      } else {
        value = boost::posix_time::duration_from_string(timeAsString);
      }
    }
  };

  template<>
  struct IsStructure<boost::posix_time::ptime> : std::false_type {};

  template<>
  struct Send<boost::posix_time::ptime> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::posix_time::ptime& value) const {
      shuttle.Shuttle(name, boost::posix_time::to_iso_string(value));
    }
  };

  template<>
  struct Receive<boost::posix_time::ptime> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::posix_time::ptime& value) const {
      auto timeAsString = std::string();
      shuttle.Shuttle(name, timeAsString);
      if(timeAsString == "+infinity") {
        value = boost::posix_time::pos_infin;
      } else if(timeAsString == "-infinity") {
        value = boost::posix_time::neg_infin;
      } else if(timeAsString == "not-a-date-time") {
        value = boost::posix_time::ptime();
      } else {
        value = boost::posix_time::from_iso_string(timeAsString);
      }
    }
  };

  template<>
  struct IsStructure<boost::gregorian::greg_weekday> : std::false_type {};

  template<>
  struct IsDefaultConstructable<boost::gregorian::greg_weekday> :
    std::false_type {};

  template<>
  inline boost::gregorian::greg_weekday DefaultConstruct<
      boost::gregorian::greg_weekday>() {
    return 0;
  }

  template<>
  struct Send<boost::gregorian::greg_weekday> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_weekday value) const {
      shuttle.Shuttle(name, static_cast<std::uint8_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_weekday> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_weekday& value) const {
      auto day = std::uint8_t();
      shuttle.Shuttle(name, day);
      value = day;
    }
  };

  template<>
  struct IsStructure<boost::gregorian::greg_day> : std::false_type {};

  template<>
  struct IsDefaultConstructable<boost::gregorian::greg_day> :
    std::false_type {};

  template<>
  inline boost::gregorian::greg_day DefaultConstruct<
      boost::gregorian::greg_day>() {
    return 0;
  }

  template<>
  struct Send<boost::gregorian::greg_day> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_day value) const {
      shuttle.Shuttle(name, static_cast<std::uint8_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_day> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_day& value) const {
      auto day = std::uint8_t();
      shuttle.Shuttle(name, day);
      value = day;
    }
  };

  template<>
  struct IsStructure<boost::gregorian::greg_month> : std::false_type {};

  template<>
  struct IsDefaultConstructable<boost::gregorian::greg_month> :
    std::false_type {};

  template<>
  inline boost::gregorian::greg_month DefaultConstruct<
      boost::gregorian::greg_month>() {
    return 0;
  }

  template<>
  struct Send<boost::gregorian::greg_month> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_month value) const {
      shuttle.Shuttle(name, static_cast<std::uint8_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_month> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_month& value) const {
      auto month = std::uint8_t();
      shuttle.Shuttle(name, month);
      value = month;
    }
  };

  template<>
  struct IsStructure<boost::gregorian::greg_year> : std::false_type {};

  template<>
  struct IsDefaultConstructable<boost::gregorian::greg_year> :
    std::false_type {};

  template<>
  inline boost::gregorian::greg_year DefaultConstruct<
      boost::gregorian::greg_year>() {
    return 1900;
  }

  template<>
  struct Send<boost::gregorian::greg_year> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_year value) const {
      shuttle.Shuttle(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_year> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::greg_year& value) const {
      auto year = std::uint16_t();
      shuttle.Shuttle(name, year);
      value = year;
    }
  };

  template<>
  struct IsStructure<boost::gregorian::date> : std::false_type {};

  template<>
  struct Send<boost::gregorian::date> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::gregorian::date& value) const {
      shuttle.Shuttle(name, boost::gregorian::to_iso_string(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::date> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::gregorian::date& value) const {
      auto dateAsString = std::string();
      shuttle.Shuttle(name, dateAsString);
      if(dateAsString == "+infinity") {
        value = boost::gregorian::date(boost::gregorian::pos_infin);
      } else if(dateAsString == "-infinity") {
        value = boost::gregorian::date(boost::gregorian::neg_infin);
      } else if(dateAsString == "not-a-date-time") {
        value = boost::gregorian::date();
      } else {
        value = boost::gregorian::from_undelimited_string(dateAsString);
      }
    }
  };
}

#endif
