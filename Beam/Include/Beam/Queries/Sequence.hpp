#ifndef BEAM_SEQUENCE_HPP
#define BEAM_SEQUENCE_HPP
#include <cstdint>
#include <limits>
#include <ostream>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Queries {

  /*! \class Sequence
      \brief Used to order sequential data.
   */
  class Sequence {
    public:

      //! The type used for the Sequence's ordinal.
      using Ordinal = std::uint64_t;

      //! Returns the first possible Sequence.
      static Sequence First();

      //! Returns the last possible Sequence.
      static Sequence Last();

      //! Returns a Sequence that can be used to represent the 'Present' in a
      //! query.
      static Sequence Present();

      //! Constructs a Sequence representing the first element.
      Sequence();

      //! Constructs a Sequence with a specified ordinal and an offset of zero.
      /*!
        \param ordinal The Sequence's ordinal.
      */
      explicit Sequence(Ordinal ordinal);

      //! Returns the ordinal.
      Ordinal GetOrdinal() const;

      //! Compares whether this Sequence comes before another.
      /*!
        \param sequence The Sequence to compare to.
        \return <code>true</code> iff <code>this</code> comes before
                <i>sequence</i>.
      */
      bool operator <(const Sequence& sequence) const;

      //! Compares whether this Sequence comes before or is equal to another.
      /*!
        \param sequence The Sequence to compare to.
        \return <code>true</code> iff <code>this</code> comes before or is equal
                to <i>sequence</i>.
      */
      bool operator <=(const Sequence& sequence) const;

      //! Compares whether this Sequence is equal to another.
      /*!
        \param sequence The Sequence to compare to.
        \return <code>true</code> iff <code>this</code> is equal to
                <i>sequence</i>.
      */
      bool operator ==(const Sequence& sequence) const;

      //! Compares whether this Sequence is not equal to another.
      /*!
        \param sequence The Sequence to compare to.
        \return <code>true</code> iff <code>this</code>is not equal to
                <i>sequence</i>.
      */
      bool operator !=(const Sequence& sequence) const;

      //! Compares whether this Sequence comes after another.
      /*!
        \param sequence The Sequence to compare to.
        \return <code>true</code> iff <code>this</code> comes after
                <i>sequence</i>.
      */
      bool operator >(const Sequence& sequence) const;

      //! Compares whether this Sequence comes after or is equal to another.
      /*!
        \param sequence The Sequence to compare to.
        \return <code>true</code> iff <code>this</code> comes after or is equal
                to <i>sequence</i>.
      */
      bool operator >=(const Sequence& sequence) const;

    private:
      Ordinal m_ordinal;
  };

  //! Returns the immediately following Sequence.
  /*!
    \param sequence The Sequence to increment.
    \return The Sequence that immediately follows <i>sequence</i>.
  */
  inline Sequence Increment(const Sequence& sequence) {
    if(sequence == Sequence::Last()) {
      return sequence;
    }
    return Sequence(sequence.GetOrdinal() + 1);
  }

  //! Increments a Sequence.
  /*!
    \param sequence The Sequence to increment.
  */
  inline Sequence& operator ++(Sequence& sequence) {
    sequence = Increment(sequence);
    return sequence;
  }

  //! Increments a Sequence.
  /*!
    \param sequence The Sequence to increment.
  */
  inline Sequence operator ++(Sequence& sequence, int) {
    auto initialValue = sequence;
    sequence = Increment(sequence);
    return initialValue;
  }

  //! Returns the immediately preceding Sequence.
  /*!
    \param sequence The Sequence to decrement.
    \return The Sequence that immediately precedes <i>sequence</i>.
  */
  inline Sequence Decrement(const Sequence& sequence) {
    if(sequence == Sequence::First()) {
      return sequence;
    }
    return Sequence(sequence.GetOrdinal() - 1);
  }

  inline std::ostream& operator <<(std::ostream& out,
      const Sequence& sequence) {
    return out << sequence.GetOrdinal();
  }

  inline Sequence Sequence::First() {
    return Sequence(0);
  }

  inline Sequence Sequence::Last() {
    return Sequence(std::numeric_limits<Ordinal>::max());
  }

  inline Sequence Sequence::Present() {
    return Decrement(Last());
  }

  inline Sequence::Sequence()
      : m_ordinal(0) {}

  inline Sequence::Sequence(Ordinal ordinal)
      : m_ordinal(ordinal) {}

  inline Sequence::Ordinal Sequence::GetOrdinal() const {
    return m_ordinal;
  }

  inline bool Sequence::operator <(const Sequence& sequence) const {
    return m_ordinal < sequence.m_ordinal;
  }

  inline bool Sequence::operator <=(const Sequence& sequence) const {
    return m_ordinal <= sequence.m_ordinal;
  }

  inline bool Sequence::operator ==(const Sequence& sequence) const {
    return m_ordinal == sequence.m_ordinal;
  }

  inline bool Sequence::operator !=(const Sequence& sequence) const {
    return !(*this == sequence);
  }

  inline bool Sequence::operator >(const Sequence& sequence) const {
    return m_ordinal > sequence.m_ordinal;
  }

  inline bool Sequence::operator >=(const Sequence& sequence) const {
    return m_ordinal >= sequence.m_ordinal;
  }
}
}

namespace Beam {
namespace Serialization {
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
      Queries::Sequence::Ordinal ordinal;
      shuttle.Shuttle("ordinal", ordinal);
      value = Queries::Sequence(ordinal);
    }
  };
}
}

#endif
