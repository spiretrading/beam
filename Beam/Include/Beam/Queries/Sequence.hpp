#ifndef BEAM_SEQUENCE_HPP
#define BEAM_SEQUENCE_HPP
#include <climits>
#include <cstdint>
#include <functional>
#include <limits>
#include <ostream>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {

  /** Used to order sequential data. */
  class Sequence {
    public:

      /** The type used for the Sequence's ordinal. */
      using Ordinal = std::uint64_t;

      /** Returns the first possible Sequence. */
      static const Sequence FIRST;

      /** Returns the last possible Sequence. */
      static const Sequence LAST;

      /**
       * Returns a Sequence that can be used to represent the 'present' in a
       * query.
       */
      static const Sequence PRESENT;

      /** Constructs a Sequence representing the first element. */
      Sequence() noexcept;

      /**
       * Constructs a Sequence with a specified ordinal and an offset of zero.
       * @param ordinal The Sequence's ordinal.
       */
      explicit Sequence(Ordinal ordinal) noexcept;

      /** Returns the ordinal. */
      Ordinal get_ordinal() const noexcept;

      auto operator <=>(const Sequence& sequence) const = default;

    private:
      Ordinal m_ordinal;
  };

  /**
   * Returns the immediately following Sequence.
   * @param sequence The Sequence to increment.
   * @return The Sequence that immediately follows <i>sequence</i>.
   */
  inline Sequence increment(Sequence sequence) noexcept {
    if(sequence == Sequence::LAST) {
      return sequence;
    }
    return Sequence(sequence.get_ordinal() + 1);
  }

  /**
   * Increments a Sequence.
   * @param sequence The Sequence to increment.
   */
  inline Sequence& operator ++(Sequence& sequence) noexcept {
    sequence = increment(sequence);
    return sequence;
  }

  /**
   * Increments a Sequence.
   * @param sequence The Sequence to increment.
   */
  inline Sequence operator ++(Sequence& sequence, int) noexcept {
    auto initial = sequence;
    sequence = increment(sequence);
    return initial;
  }

  /**
   * Returns the immediately preceding Sequence.
   * @param sequence The Sequence to decrement.
   * @return The Sequence that immediately precedes <i>sequence</i>.
   */
  inline Sequence decrement(Sequence sequence) noexcept {
    if(sequence == Sequence::FIRST) {
      return sequence;
    }
    return Sequence(sequence.get_ordinal() - 1);
  }

  /**
   * Encodes a timestamp into a Sequence.
   * @param timestamp The timestamp to encode.
   * @return The Sequence number with the <i>timestamp</i> encoded into it.
   */
  inline Sequence to_sequence(boost::posix_time::ptime timestamp) {
    const auto YEAR_SIZE = 13;
    const auto MONTH_SIZE = 4;
    const auto DAY_SIZE = 6;
    const auto ENCODING_SIZE = YEAR_SIZE + MONTH_SIZE + DAY_SIZE;
    if(timestamp == boost::posix_time::neg_infin) {
      return Sequence::FIRST;
    } else if(timestamp == boost::posix_time::pos_infin) {
      return Sequence::LAST;
    } else if(timestamp.is_not_a_date_time()) {
      return Sequence(0);
    } else if(timestamp.is_special()) {
      boost::throw_with_location(std::out_of_range("Invalid timestamp."));
    } else {
      auto year = static_cast<Sequence::Ordinal>(timestamp.date().year()) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) - YEAR_SIZE);
      auto month = static_cast<Sequence::Ordinal>(timestamp.date().month()) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) - (YEAR_SIZE + MONTH_SIZE));
      auto day = static_cast<Sequence::Ordinal>(timestamp.date().day()) <<
        (CHAR_BIT * sizeof(Sequence::Ordinal) -
        (YEAR_SIZE + MONTH_SIZE + DAY_SIZE));
      return Sequence(year + month + day);
    }
  }

  /**
   * Encodes a timestamp into a Sequence.
   * @param timestamp The timestamp to encode.
   * @return The Sequence number with the <i>timestamp</i> encoded into it.
   */
  inline Sequence encode(
      boost::posix_time::ptime timestamp, Sequence sequence) {
    if(timestamp == boost::posix_time::neg_infin) {
      return Sequence::FIRST;
    } else if(timestamp == boost::posix_time::pos_infin) {
      return Sequence::LAST;
    } else if(timestamp.is_not_a_date_time()) {
      return sequence;
    } else {
      return Sequence(
        to_sequence(timestamp).get_ordinal() + sequence.get_ordinal());
    }
  }

  /**
   * Decodes the timestamp within a Sequence.
   * @param sequence The Sequence to decode.
   * @return The timestamp encoded within the <i>sequence</i>
   */
  inline boost::posix_time::ptime decode_timestamp(Sequence sequence) {
    const auto YEAR_SIZE = 13;
    const auto MONTH_SIZE = 4;
    const auto DAY_SIZE = 6;
    const auto ENCODING_SIZE = YEAR_SIZE + MONTH_SIZE + DAY_SIZE;
    if(sequence == Sequence::FIRST) {
      return boost::posix_time::neg_infin;
    } else if(sequence == Sequence::LAST) {
      return boost::posix_time::pos_infin;
    } else if(sequence == Sequence::PRESENT) {
      return boost::posix_time::not_a_date_time;
    } else {
      auto date = static_cast<std::uint32_t>(
        ((static_cast<Sequence::Ordinal>(-1) <<
          (CHAR_BIT * sizeof(Sequence::Ordinal) - ENCODING_SIZE)) &
            sequence.get_ordinal()) >>
              (CHAR_BIT * sizeof(Sequence::Ordinal) - ENCODING_SIZE));
      auto day = static_cast<unsigned short>(
        ~(static_cast<std::uint32_t>(-1) << DAY_SIZE) & date);
      date >>= DAY_SIZE;
      auto month = static_cast<unsigned short>(
        ~(static_cast<std::uint32_t>(-1) << MONTH_SIZE) & date);
      date >>= MONTH_SIZE;
      auto year = static_cast<unsigned short>(
        ~(static_cast<std::uint32_t>(-1) << YEAR_SIZE) & date);
      if(year == 0) {
        return boost::posix_time::not_a_date_time;
      }
      return boost::posix_time::ptime(boost::gregorian::date(year, month, day),
        boost::posix_time::seconds(0));
    }
  }

  inline std::ostream& operator <<(std::ostream& out, Sequence sequence) {
    return out << sequence.get_ordinal();
  }

  inline std::size_t hash_value(Sequence sequence) noexcept {
    return static_cast<std::size_t>(sequence.get_ordinal());
  }

  inline const Sequence Sequence::FIRST = Sequence(0);

  inline const Sequence Sequence::LAST =
    Sequence(std::numeric_limits<Ordinal>::max());

  inline const Sequence Sequence::PRESENT = decrement(LAST);

  inline Sequence::Sequence() noexcept
    : m_ordinal(0) {}

  inline Sequence::Sequence(Ordinal ordinal) noexcept
    : m_ordinal(ordinal) {}

  inline Sequence::Ordinal Sequence::get_ordinal() const noexcept {
    return m_ordinal;
  }

  template<>
  constexpr auto is_structure<Sequence> = false;

  template<>
  struct Send<Sequence> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, const Sequence& value) const {
      sender.send(name, value.get_ordinal());
    }
  };

  template<>
  struct Receive<Sequence> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, Sequence& value) const {
      value = Sequence(receive<Sequence::Ordinal>(receiver, name));
    }
  };
}

namespace std {
  template<>
  struct hash<Beam::Sequence> {
    std::size_t operator ()(Beam::Sequence value) const noexcept {
      return Beam::hash_value(value);
    }
  };
}

#endif
