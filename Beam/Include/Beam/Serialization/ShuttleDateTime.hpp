#ifndef BEAM_SHUTTLE_DATE_TIME_HPP
#define BEAM_SHUTTLE_DATE_TIME_HPP
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<>
  constexpr auto is_structure<boost::posix_time::time_duration> = false;

  template<>
  struct Send<boost::posix_time::time_duration> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        boost::posix_time::time_duration value) const {
      sender.send(name, boost::posix_time::to_simple_string(value));
    }
  };

  template<>
  struct Receive<boost::posix_time::time_duration> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::posix_time::time_duration& value) const {
      auto time_as_string = receive<std::string>(receiver, name);
      if(time_as_string == "+infinity") {
        value = boost::posix_time::pos_infin;
      } else if(time_as_string == "-infinity") {
        value = boost::posix_time::neg_infin;
      } else {
        value = boost::posix_time::duration_from_string(time_as_string);
      }
    }
  };

  template<>
  constexpr auto is_structure<boost::posix_time::ptime> = false;

  template<>
  struct Send<boost::posix_time::ptime> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, boost::posix_time::ptime value) const {
      sender.send(name, boost::posix_time::to_iso_string(value));
    }
  };

  template<>
  struct Receive<boost::posix_time::ptime> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, boost::posix_time::ptime& value) const {
      auto time_as_string = receive<std::string>(receiver, name);
      if(time_as_string == "+infinity") {
        value = boost::posix_time::pos_infin;
      } else if(time_as_string == "-infinity") {
        value = boost::posix_time::neg_infin;
      } else if(time_as_string == "not-a-date-time") {
        value = boost::posix_time::ptime();
      } else {
        value = boost::posix_time::from_iso_string(time_as_string);
      }
    }
  };

  template<>
  constexpr auto is_structure<boost::gregorian::greg_weekday> = false;

  template<>
  inline boost::gregorian::greg_weekday
      default_construct<boost::gregorian::greg_weekday>() {
    return 0;
  }

  template<>
  struct Send<boost::gregorian::greg_weekday> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        boost::gregorian::greg_weekday value) const {
      sender.send(name, static_cast<std::uint8_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_weekday> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::gregorian::greg_weekday& value) const {
      value = receive<std::uint8_t>(receiver, name);
    }
  };

  template<>
  constexpr auto is_structure<boost::gregorian::greg_day> = false;

  template<>
  inline boost::gregorian::greg_day
      default_construct<boost::gregorian::greg_day>() {
    return 1;
  }

  template<>
  struct Send<boost::gregorian::greg_day> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, boost::gregorian::greg_day value) const {
      sender.send(name, static_cast<std::uint8_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_day> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::gregorian::greg_day& value) const {
      value = receive<std::uint8_t>(receiver, name);
    }
  };

  template<>
  constexpr auto is_structure<boost::gregorian::greg_month> = false;

  template<>
  inline boost::gregorian::greg_month
      default_construct<boost::gregorian::greg_month>() {
    return 1;
  }

  template<>
  struct Send<boost::gregorian::greg_month> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        boost::gregorian::greg_month value) const {
      sender.send(name, static_cast<std::uint8_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_month> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::gregorian::greg_month& value) const {
      value = receive<std::uint8_t>(receiver, name);
    }
  };

  template<>
  constexpr auto is_structure<boost::gregorian::greg_year> = false;

  template<>
  inline boost::gregorian::greg_year
      default_construct<boost::gregorian::greg_year>() {
    return 1900;
  }

  template<>
  struct Send<boost::gregorian::greg_year> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        boost::gregorian::greg_year value) const {
      sender.send(name, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::greg_year> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::gregorian::greg_year& value) const {
      value = receive<std::uint16_t>(receiver, name);
    }
  };

  template<>
  constexpr auto is_structure<boost::gregorian::date> = false;

  template<>
  struct Send<boost::gregorian::date> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        const boost::gregorian::date& value) const {
      sender.send(name, boost::gregorian::to_iso_string(value));
    }
  };

  template<>
  struct Receive<boost::gregorian::date> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::gregorian::date& value) const {
      auto date_as_string = receive<std::string>(receiver, name);
      if(date_as_string == "+infinity") {
        value = boost::gregorian::date(boost::gregorian::pos_infin);
      } else if(date_as_string == "-infinity") {
        value = boost::gregorian::date(boost::gregorian::neg_infin);
      } else if(date_as_string == "not-a-date-time") {
        value = boost::gregorian::date();
      } else {
        value = boost::gregorian::from_undelimited_string(date_as_string);
      }
    }
  };
}

#endif
