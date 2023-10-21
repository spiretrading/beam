#ifndef BEAM_SEQUENCE_HPP
#define BEAM_SEQUENCE_HPP
#include <climits>
#include <cstdint>
#include <functional>
#include <limits>
#include <ostream>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam::Queries {

  /** Used to order sequential data. */
  class Sequence {
    public:

      /** The type used for the Sequence's ordinal. */
      using Ordinal = std::uint64_t;

      /** Returns the first possible Sequence. */
      static Sequence First() noexcept;

      /** Returns the last possible Sequence. */
      static Sequence Last() noexcept;

      /**
       * Returns a Sequence that can be used to represent the 'Present' in a
       * query.
       */
      static Sequence Present() noexcept;

      /** Constructs a Sequence representing the first element. */
      Sequence() noexcept;

      /**
       * Constructs a Sequence with a specified ordinal and an offset of zero.
       * @param ordinal The Sequence's ordinal.
       */
      explicit Sequence(Ordinal ordinal) noexcept;

      /** Returns the ordinal. */
      Ordinal GetOrdinal() const noexcept;

      auto operator <=>(const Sequence& sequence) const = default;

    private:
      Ordinal m_ordinal;
  };

  /**
   * Returns the immediately following Sequence.
   * @param sequence The Sequence to increment.
   * @return The Sequence that immediately follows <i>sequence</i>.
   */
  inline Sequence Increment(const Sequence& sequence) noexcept {
    if(sequence == Sequence::Last()) {
      return sequence;
    }
    return Sequence(sequence.GetOrdinal() + 1);
  }

  /**
   * Increments a Sequence.
   * @param sequence The Sequence to increment.
   */
  inline Sequence& operator ++(Sequence& sequence) noexcept {
    sequence = Increment(sequence);
    return sequence;
  }

  /**
   * Increments a Sequence.
   * @param sequence The Sequence to increment.
   */
  inline Sequence operator ++(Sequence& sequence, int) noexcept {
    auto initialValue = sequence;
    sequence = Increment(sequence);
    return initialValue;
  }

  /**
   * Returns the immediately preceding Sequence.
   * @param sequence The Sequence to decrement.
   * @return The Sequence that immediately precedes <i>sequence</i>.
   */
  inline Sequence Decrement(const Sequence& sequence) noexcept {
    if(sequence == Sequence::First()) {
      return sequence;
    }
    return Sequence(sequence.GetOrdinal() - 1);
  }

  /**
   * Encodes a timestamp into a Sequence.
   * @param timestamp The timestamp to encode.
   * @return The Sequence number with the <i>timestamp</i> encoded into it.
   */
  inline Sequence EncodeTimestamp(boost::posix_time::ptime timestamp) {
    const auto YEAR_SIZE = 13;
    const auto MONTH_SIZE = 4;
    const auto DAY_SIZE = 6;
    const auto ENCODING_SIZE = YEAR_SIZE + MONTH_SIZE + DAY_SIZE;
    if(timestamp == boost::posix_time::neg_infin) {
      return Sequence::First();
    } else if(timestamp == boost::posix_time::pos_infin) {
      return Sequence::Last();
    } else if(timestamp.is_not_a_date_time()) {
      return Sequence(0);
    } else if(timestamp.is_special()) {
      BOOST_THROW_EXCEPTION(std::out_of_range("Invalid timestamp."));
    } else {
      auto yearComponent =
        static_cast<Sequence::Ordinal>(timestamp.date().year()) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) - YEAR_SIZE);
      auto monthComponent =
        static_cast<Sequence::Ordinal>(timestamp.date().month()) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) - (YEAR_SIZE + MONTH_SIZE));
      auto dayComponent =
        static_cast<Sequence::Ordinal>(timestamp.date().day()) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) -
        (YEAR_SIZE + MONTH_SIZE + DAY_SIZE));
      return Sequence(yearComponent + monthComponent + dayComponent);
    }
  }

  /**
   * Encodes a timestamp into a Sequence.
   * @param timestamp The timestamp to encode.
   * @return The Sequence number with the <i>timestamp</i> encoded into it.
   */
  inline Sequence EncodeTimestamp(boost::posix_time::ptime timestamp,
      Sequence sequence) {
    if(timestamp == boost::posix_time::neg_infin) {
      return Sequence::First();
    } else if(timestamp == boost::posix_time::pos_infin) {
      return Sequence::Last();
    } else if(timestamp.is_not_a_date_time()) {
      return sequence;
    } else {
      return Sequence(
        EncodeTimestamp(timestamp).GetOrdinal() + sequence.GetOrdinal());
    }
  }

  /**
   * Decodes the timestamp within a Sequence.
   * @param sequence The Sequence to decode.
   * @return The timestamp encoded within the <i>sequence</i>
   */
  inline boost::posix_time::ptime DecodeTimestamp(Sequence sequence) {
    const auto YEAR_SIZE = 13;
    const auto MONTH_SIZE = 4;
    const auto DAY_SIZE = 6;
    const auto ENCODING_SIZE = YEAR_SIZE + MONTH_SIZE + DAY_SIZE;
    if(sequence == Sequence::First()) {
      return boost::posix_time::neg_infin;
    } else if(sequence == Sequence::Last()) {
      return boost::posix_time::pos_infin;
    } else if(sequence == Sequence::Present()) {
      return boost::posix_time::not_a_date_time;
    } else {
      auto dateEncoding = static_cast<std::uint32_t>(
        ((static_cast<Sequence::Ordinal>(-1) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) - ENCODING_SIZE)) &
        sequence.GetOrdinal()) >>
        (CHAR_BIT * sizeof(Sequence::Ordinal) - ENCODING_SIZE));
      auto day = static_cast<unsigned short>(
        ~(static_cast<std::uint32_t>(-1) << DAY_SIZE) & dateEncoding);
      dateEncoding >>= DAY_SIZE;
      auto month = static_cast<unsigned short>(
        ~(static_cast<std::uint32_t>(-1) << MONTH_SIZE) & dateEncoding);
      dateEncoding >>= MONTH_SIZE;
      auto year = static_cast<unsigned short>(
        ~(static_cast<std::uint32_t>(-1) << YEAR_SIZE) & dateEncoding);
      if(year == 0) {
        return boost::posix_time::not_a_date_time;
      }
      return boost::posix_time::ptime(boost::gregorian::date(year, month, day),
        boost::posix_time::seconds(0));
    }
  }

  inline std::ostream& operator <<(std::ostream& out, Sequence sequence) {
    return out << sequence.GetOrdinal();
  }

  inline std::size_t hash_value(Sequence sequence) noexcept {
    return static_cast<std::size_t>(sequence.GetOrdinal());
  }

  inline Sequence Sequence::First() noexcept {
    return Sequence(0);
  }

  inline Sequence Sequence::Last() noexcept {
    return Sequence(std::numeric_limits<Ordinal>::max());
  }

  inline Sequence Sequence::Present() noexcept {
    return Decrement(Last());
  }

  inline Sequence::Sequence() noexcept
    : m_ordinal(0) {}

  inline Sequence::Sequence(Ordinal ordinal) noexcept
    : m_ordinal(ordinal) {}

  inline Sequence::Ordinal Sequence::GetOrdinal() const noexcept {
    return m_ordinal;
  }
}

namespace Beam::Serialization {
  template<>
  struct Send<Queries::Sequence> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const Queries::Sequence& value,
        unsigned int version) {
      shuttle.Shuttle("ordinal", value.GetOrdinal());
    }
  };

  template<>
  struct Receive<Queries::Sequence> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::Sequence& value,
        unsigned int version) {
      auto ordinal = Queries::Sequence::Ordinal();
      shuttle.Shuttle("ordinal", ordinal);
      value = Queries::Sequence(ordinal);
    }
  };
}

namespace std {
  template<>
  struct hash<Beam::Queries::Sequence> {
    std::size_t operator ()(Beam::Queries::Sequence value) const noexcept {
      return Beam::Queries::hash_value(value);
    }
  };
}

#endif
