#ifndef BEAM_SEQUENCER_HPP
#define BEAM_SEQUENCER_HPP
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {

  /** Produces SequencedValues encoding the timestamp into the Sequence. */
  class Sequencer {
    public:

      /**
       * Constructs a Sequencer.
       * @param initial The Sequence to use on the first value.
       */
      explicit Sequencer(Sequence initial) noexcept;

      /**
       * Makes a SequencedValue encoding the value's timestamp into the
       * Sequence.
       * @param value The SequencedValue's value.
       * @param index The SequencedValue's index.
       * @return A SequencedValue representing the <i>value</i> and <i>index</i>
       *         with a Sequence encoding the <i>value</i>'s timestamp.
       */
      template<typename Value, typename Index>
      SequencedValue<IndexedValue<Value, Index>> make_sequenced_value(
        Value value, Index index);

      /**
       * Returns the next Sequence to use for a specified timestamp.
       * @param timestamp The timestamp to translate into a Sequence.
       * @return The next Sequence to use for the specified <i>timestamp</i>.
       */
      Sequence increment_next_sequence(boost::posix_time::ptime timestamp);

    private:
      Sequence m_next_sequence;
      boost::posix_time::ptime m_partition;

      Sequencer(const Sequencer&) = delete;
      Sequencer& operator =(const Sequencer&) = delete;
  };

  /**
   * Returns the timestamp representing the partition that a timestamp belongs
   * to. Currently the partition is strictly the timestamp's date.
   * @param timestamp The timestamp whose partition is to be returned.
   * @return A timestamp representing the partition that the <i>timestamp</i>
   *         belongs to, which is currently the date.
   */
  inline boost::posix_time::ptime get_partition(
      boost::posix_time::ptime timestamp) {
    return boost::posix_time::ptime(timestamp.date());
  }

  /**
   * Tests if a timestamp is part of a partition.
   * @param timestamp The timestamp to test.
   * @param partition The timestamp representing the partition.
   * @return <code>true</code> iff the <i>timestamp</i> belongs to the partition
   *         represented by <i>partition</i>.
   */
  inline bool is_same_partition(
      boost::posix_time::ptime timestamp, boost::posix_time::ptime partition) {
    return get_partition(timestamp) <= get_partition(partition);
  }

  /**
   * Removes the index from a value containing both a sequence and an index.
   * @param value The value whose index is to be removed.
   */
  template<typename Value, typename Index>
  SequencedValue<Value> to_sequenced_value(
      SequencedValue<IndexedValue<Value, Index>> value) {
    return SequencedValue(**value, value.get_sequence());
  }

  inline Sequencer::Sequencer(Sequence initial) noexcept
    : m_next_sequence(initial),
      m_partition(decode_timestamp(m_next_sequence)) {}

  template<typename Value, typename Index>
  SequencedValue<IndexedValue<Value, Index>>
      Sequencer::make_sequenced_value(Value value, Index index) {
    auto sequence = increment_next_sequence(get_timestamp(value));
    return SequencedValue(
      IndexedValue(std::move(value), std::move(index)), sequence);
  }

  inline Sequence Sequencer::increment_next_sequence(
      boost::posix_time::ptime timestamp) {
    if(!is_same_partition(timestamp, m_partition)) {
      m_partition = get_partition(timestamp);
      m_next_sequence = to_sequence(timestamp);
    }
    auto sequence = m_next_sequence;
    ++m_next_sequence;
    return sequence;
  }
}

#endif
