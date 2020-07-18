#ifndef BEAM_QUERYRANGE_HPP
#define BEAM_QUERYRANGE_HPP
#include <ostream>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/variant.hpp>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Serialization/ShuttleVariant.hpp"

namespace Beam {
namespace Queries {

  /*! \class Range
      \brief Defines the range of values to Query over in terms of time or
             Sequence points.
   */
  class Range {
    public:

      //! Stores a single end-point in a range as either a time or a Sequence.
      using Point = boost::variant<Sequence, boost::posix_time::ptime>;

      //! Returns an empty Range.
      static Range Empty();

      //! Returns a Range representing all historical Sequences.
      static Range Historical();

      //! Returns a Range representing the entire Sequence including real time
      //! data.
      static Range Total();

      //! Returns a Range representing real time data.
      static Range RealTime();

      //! Constructs an empty Range.
      Range();

      //! Constructs a Range given two points.
      /*!
        \param start The start of the Range.
        \param end The end of the Range.
      */
      Range(const Point& start, const Point& end);

      //! Constructs a Range given two points.
      /*!
        \param start The start of the Range.
        \param end The end of the Range.
      */
      Range(boost::posix_time::special_values start, const Point& end);

      //! Constructs a Range given two points.
      /*!
        \param start The start of the Range.
        \param end The end of the Range.
      */
      Range(const Point& start, boost::posix_time::special_values end);

      //! Constructs a Range given two points.
      /*!
        \param start The start of the Range.
        \param end The end of the Range.
      */
      Range(boost::posix_time::special_values start,
        boost::posix_time::special_values end);

      //! Returns the start of the Range.
      const Point& GetStart() const;

      //! Returns the end of the Range.
      const Point& GetEnd() const;

      //! Checks if two Ranges are equal.
      /*!
        \param range The Range to check for equality.
        \return <code>true</code> iff <code>this</code> is equal to
                <i>range</i>.
      */
      bool operator ==(const Range& range) const;

      //! Checks if two Ranges are not equal.
      /*!
        \param range The Range to check for inequality.
        \return <code>true</code> iff <code>this</code> is not equal to
                <i>range</i>.
      */
      bool operator !=(const Range& range) const;

    private:
      Point m_start;
      Point m_end;

      static bool IsValid(const Point& point);
      static Point Validate(const Point& point);
      void Initialize(const Point& start, const Point& end);
  };

  /*! \struct TimestampAccessor
      \brief Provides access to an object's timestamp field.
      \tparam T The type of object to access.
   */
  template<typename T>
  struct TimestampAccessor {
    const boost::posix_time::ptime& operator ()(const T& value) const {
      return value.m_timestamp;
    }

    boost::posix_time::ptime& operator ()(T& value) const {
      return value.m_timestamp;
    }
  };

  //! Returns a value's timestamp.
  /*!
    \param value The value to get the timestamp from.
    \return The <i>value</i>'s timestamp.
  */
  template<typename T>
  boost::posix_time::ptime& GetTimestamp(T& value) {
    return TimestampAccessor<T>()(value);
  }

  //! Returns a value's timestamp.
  /*!
    \param value The value to get the timestamp from.
    \return The <i>value</i>'s timestamp.
  */
  template<typename T>
  const boost::posix_time::ptime& GetTimestamp(const T& value) {
    return TimestampAccessor<T>()(value);
  }

  /*! \struct TimestampComparator
      \brief Defines a comparator for any two timestamped values.
   */
  struct TimestampComparator {
    template<typename T, typename Q>
    bool operator ()(const T& lhs, const Q& rhs) const {
      return GetTimestamp(lhs) < GetTimestamp(rhs);
    }
  };

  //! Tests if a value comes after a given Range Point.
  /*!
    \param value The value to test.
    \param point The Range Point to compare the value to.
    \return <code>true</code> iff the value comes after the specified
            <i>point</i>.
  */
  template<typename T>
  bool RangePointLesserOrEqual(const T& value, const Range::Point& point) {
    if(auto sequence = boost::get<Sequence>(&point)) {
      return value.GetSequence() <= *sequence;
    }
    auto& timestamp = boost::get<boost::posix_time::ptime>(point);
    return GetTimestamp(value) <= timestamp;
  }

  //! Tests if a value comes before a given Range Point.
  /*!
    \param value The value to test.
    \param point The Range Point to compare the value to.
    \return <code>true</code> iff the value comes before the specified
            <i>point</i>.
  */
  template<typename T>
  bool RangePointGreaterOrEqual(const T& value, const Range::Point& point) {
    if(auto sequence = boost::get<Sequence>(&point)) {
      return value.GetSequence() >= *sequence;
    }
    auto& timestamp = boost::get<boost::posix_time::ptime>(point);
    return GetTimestamp(value) >= timestamp;
  }

  inline std::ostream& operator <<(std::ostream& out, const Range& range) {
    if(range == Range::Empty()) {
      return out << "Empty";
    } else if(range == Range::Total()) {
      return out << "Total";
    }
    return out << "(" << range.GetStart() << " " << range.GetEnd() << ")";
  }

  inline bool operator ==(const Range::Point& range, const Sequence& sequence) {
    return boost::get<const Sequence>(&range) &&
      boost::get<Sequence>(range) == sequence;
  }

  inline bool operator !=(const Range::Point& range, const Sequence& sequence) {
    return !(range == sequence);
  }

  inline bool operator ==(const Range::Point& range,
      const boost::posix_time::ptime& time) {
    return boost::get<const boost::posix_time::ptime>(&range) &&
      boost::get<boost::posix_time::ptime>(range) == time;
  }

  inline bool operator !=(const Range::Point& range,
      const boost::posix_time::ptime& time) {
    return !(range == time);
  }

  inline Range Range::Empty() {
    return Range(Sequence::First(), Sequence::First());
  }

  inline Range Range::Historical() {
    return Range(Sequence::First(), Sequence::Present());
  }

  inline Range Range::Total() {
    return Range(Sequence::First(), Sequence::Last());
  }

  inline Range Range::RealTime() {
    return Range(Sequence::Present(), Sequence::Last());
  }

  inline Range::Range()
      : m_start(Sequence::First()),
        m_end(Sequence::First()) {}

  inline Range::Range(const Point& start, const Point& end) {
    Initialize(start, end);
  }

  inline Range::Range(boost::posix_time::special_values start,
      const Point& end) {
    Initialize(boost::posix_time::ptime(start), end);
  }

  inline Range::Range(const Point& start,
      boost::posix_time::special_values end) {
    Initialize(start, boost::posix_time::ptime(end));
  }

  inline Range::Range(boost::posix_time::special_values start,
      boost::posix_time::special_values end) {
    Initialize(boost::posix_time::ptime(start), boost::posix_time::ptime(end));
  }

  inline const Range::Point& Range::GetStart() const {
    return m_start;
  }

  inline const Range::Point& Range::GetEnd() const {
    return m_end;
  }

  inline bool Range::operator ==(const Range& range) const {
    return m_start == range.m_start && m_end == range.m_end;
  }

  inline bool Range::operator !=(const Range& range) const {
    return !(*this == range);
  }

  inline bool Range::IsValid(const Point& point) {
    if(const boost::posix_time::ptime* time =
        boost::get<const boost::posix_time::ptime>(&point)) {
      if(time->is_special()) {
        if(*time != boost::posix_time::pos_infin &&
            *time != boost::posix_time::neg_infin) {
          return false;
        }
      }
    }
    return true;
  }

  inline Range::Point Range::Validate(const Point& point) {
    if(const boost::posix_time::ptime* pointDate =
        boost::get<const boost::posix_time::ptime>(&point)) {
      if(*pointDate == boost::posix_time::neg_infin) {
        return Sequence::First();
      } else if(*pointDate == boost::posix_time::pos_infin) {
        return Sequence::Last();
      } else {
        return point;
      }
    }
    return point;
  }

  inline void Range::Initialize(const Point& start, const Point& end) {
    if(!IsValid(start) || !IsValid(end)) {
      m_start = Sequence::First();
      m_end = Sequence::First();
      return;
    }
    m_start = Validate(start);
    m_end = Validate(end);
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Send<Beam::Queries::Range> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const Beam::Queries::Range& value,
        unsigned int version) {
      shuttle.Shuttle("start", value.GetStart());
      shuttle.Shuttle("end", value.GetEnd());
    }
  };

  template<>
  struct Receive<Beam::Queries::Range> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Beam::Queries::Range& value,
        unsigned int version) {
      Queries::Range::Point start;
      Queries::Range::Point end;
      shuttle.Shuttle("start", start);
      shuttle.Shuttle("end", end);
      value = Queries::Range(start, end);
    }
  };
}
}

#endif
