#ifndef BEAM_SHUTTLEDATETIME_HPP
#define BEAM_SHUTTLEDATETIME_HPP
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
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
      std::string timeAsString;
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
      std::string timeAsString;
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
      std::string dateAsString;
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
}

#endif
