#ifndef BEAM_TAGGED_QUEUE_HPP
#define BEAM_TAGGED_QUEUE_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /**
   * Used to tag a value pushed onto a Queue with a key/index.
   * @param K The type used to index values pushed.
   * @param V The type of values pushed onto the Queue.
   */
  template<typename K, typename V>
  class TaggedQueue : public QueueReader<KeyValuePair<K, V>> {
    public:

      /** The type used to index values pushed. */
      using Key = K;

      /** The type of values pushed onto the Queue. */
      using Value = V;

      using Target = typename QueueReader<KeyValuePair<K, V>>::Target;

      /** Constructs a TaggedQueue. */
      TaggedQueue() = default;

      /**
       * Returns a Queue linked to a specified key.
       * @param key The key used to identify values pushed onto this Queue.
       * @return A Queue that tags values pushed onto it with the specified
       *         <i>key</i>.
       */
      std::shared_ptr<CallbackQueueWriter<Value>> GetSlot(Key key);

      bool IsEmpty() const override;

      Target Top() const override;

      void Pop() override;

      void Break(const std::exception_ptr& exception) override;

      using QueueReader<KeyValuePair<K, V>>::Break;

    protected:
      bool IsAvailable() const override;

    private:
      Queue<Target> m_values;
      CallbackQueue m_callbacks;
  };

  template<typename K, typename V>
  std::shared_ptr<CallbackQueueWriter<typename TaggedQueue<K, V>::Value>>
      TaggedQueue<K, V>::GetSlot(Key key) {
    return m_callbacks.GetSlot<Value>(
      [=] (const Value& value) {
        m_values.Push(MakeKeyValuePair(key, value));
      });
  }

  template<typename K, typename V>
  bool TaggedQueue<K, V>::IsEmpty() const {
    return m_values.IsEmpty();
  }

  template<typename K, typename V>
  typename TaggedQueue<K, V>::Target TaggedQueue<K, V>::Top() const {
    return m_values.Top();
  }

  template<typename K, typename V>
  void TaggedQueue<K, V>::Pop() {
    return m_values.Pop();
  }

  template<typename K, typename V>
  void TaggedQueue<K, V>::Break(const std::exception_ptr& exception) {
    m_callbacks.Break(exception);
    m_values.Break(exception);
  }

  template<typename K, typename V>
  bool TaggedQueue<K, V>::IsAvailable() const {
    return m_values.IsAvailable();
  }
}

#endif
