#ifndef BEAM_SEQUENCER_HPP
#define BEAM_SEQUENCER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {
namespace Queries {

  /*! \class Sequencer
      \brief Produces SequencedValues encoding the timestamp into the Sequence.
   */
  class Sequencer : private boost::noncopyable {
    public:

      //! Constructs a Sequencer.
      /*!
        \param initialSequence The Sequence to use on the first value.
      */
      explicit Sequencer(Sequence initialSequence);

      //! Makes a SequencedValue encoding the value's timestamp into the
      //! Sequence.
      /*!
        \param value The SequencedValue's value.
        \param index The SequencedValue's index.
        \return A SequencedValue representing the <i>value</i> and <i>index</i>
                with a Sequence encoding the <i>value</i>'s timestamp.
      */
      template<typename Value, typename Index>
      SequencedValue<IndexedValue<Value, Index>> MakeSequencedValue(
        Value value, Index index);

      //! Returns the next Sequence to use for a specified timestamp.
      /*!
        \param timestamp The timestamp to translate into a Sequence.
        \return The next Sequence to use for the specified <i>timestamp</i>.
      */
      Sequence IncrementNextSequence(
        const boost::posix_time::ptime& timestamp);

    private:
      Sequence m_nextSequence;
      boost::posix_time::ptime m_partition;
  };

  //! Returns the timestamp representing the partition that a timestamp belongs
  //! to.  Currently the partition is strictly the timestamp's date.
  /*!
    \param timestamp The timestamp whose partition is to be returned.
    \return A timestamp representing the partition that the <i>timestamp</i>
            belongs to, which is currently the date.
  */
  inline boost::posix_time::ptime GetPartition(
      const boost::posix_time::ptime& timestamp) {
    return boost::posix_time::ptime{timestamp.date()};
  }

  //! Tests if a timestamp is part of a partition.
  /*!
    \param timestamp The timestamp to test.
    \param partition The timestamp representing the partition.
    \return <code>true</code> iff the <i>timestamp</i> belongs to the partition
            represented by <i>partition</i>.
  */
  inline bool IsSamePartition(const boost::posix_time::ptime& timestamp,
      const boost::posix_time::ptime& partition) {
    return GetPartition(timestamp) <= GetPartition(partition);
  }

  inline Sequencer::Sequencer(Sequence initialSequence)
      : m_nextSequence{initialSequence},
        m_partition{DecodeTimestamp(m_nextSequence)} {}

  template<typename Value, typename Index>
  SequencedValue<IndexedValue<Value, Index>> Sequencer::MakeSequencedValue(
      Value value, Index index) {
    auto sequence = IncrementNextSequence(GetTimestamp(value));
    return SequencedValue(IndexedValue(std::move(value), std::move(index)),
      sequence);
  }

  inline Sequence Sequencer::IncrementNextSequence(
      const boost::posix_time::ptime& timestamp) {
    if(!IsSamePartition(timestamp, m_partition)) {
      m_partition = GetPartition(timestamp);
      m_nextSequence = EncodeTimestamp(timestamp);
    }
    auto sequence = m_nextSequence;
    ++m_nextSequence;
    return sequence;
  }
}
}

#endif
