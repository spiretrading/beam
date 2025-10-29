#ifndef BEAM_TAGGED_QUEUE_READER_HPP
#define BEAM_TAGGED_QUEUE_READER_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /**
   * Used to tag a value pushed onto a QueueWriter with a key/index.
   * @param K The type used to index values pushed.
   * @param V The type of values pushed onto the queue.
   */
  template<typename K, typename V>
  class TaggedQueueReader : public QueueReader<KeyValuePair<K, V>> {
    public:

      /** The type used to index values pushed. */
      using Key = K;

      /** The type of values pushed onto the Queue. */
      using Value = V;

      using Source = typename QueueReader<KeyValuePair<K, V>>::Source;

      /** Constructs a TaggedQueueReader. */
      TaggedQueueReader() = default;

      /**
       * Returns a QueueWriter linked to a specified key.
       * @param key The key used to identify values pushed onto this Queue.
       * @return A QueueWriter that tags values pushed onto it with the
       *         specified <i>key</i>.
       */
      auto get_slot(Key key);

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& exception) override;
      using QueueReader<KeyValuePair<K, V>>::close;

    private:
      Queue<Source> m_values;
      CallbackQueue m_callbacks;
  };

  template<typename K, typename V>
  auto TaggedQueueReader<K, V>::get_slot(Key key) {
    return m_callbacks.get_slot<Value>(
      [=, key = std::move(key), this] (const Value& value) {
        m_values.push(KeyValuePair(key, value));
      });
  }

  template<typename K, typename V>
  typename TaggedQueueReader<K, V>::Source TaggedQueueReader<K, V>::pop() {
    return m_values.pop();
  }

  template<typename K, typename V>
  boost::optional<typename TaggedQueueReader<K, V>::Source>
      TaggedQueueReader<K, V>::try_pop() {
    return m_values.try_pop();
  }

  template<typename K, typename V>
  void TaggedQueueReader<K, V>::close(const std::exception_ptr& exception) {
    m_callbacks.close(exception);
    m_values.close(exception);
  }
}

#endif
