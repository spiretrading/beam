#ifndef BEAM_TAGGEDQUEUE_HPP
#define BEAM_TAGGEDQUEUE_HPP
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"

namespace Beam {

  /*! \class TaggedQueue
      \brief Used to tag a value pushed onto a Queue with a key/index.
      \tparam KeyType The type used to index values pushed.
      \tparam ValueType The type of values pushed onto the Queue.
   */
  template<typename KeyType, typename ValueType>
  class TaggedQueue : public QueueReader<KeyValuePair<KeyType, ValueType>> {
    public:

      //! The type used to index values pushed.
      using Key = KeyType;

      //! The type of values pushed onto the Queue.
      using Value = ValueType;

      using Target = typename QueueReader<
        KeyValuePair<KeyType, ValueType>>::Target;

      //! Constructs a TaggedQueue.
      TaggedQueue() = default;

      virtual ~TaggedQueue();

      //! Returns a Queue linked to a specified key.
      /*!
        \param key The key used to identify values pushed onto this Queue.
        \return A Queue that tags values pushed onto it with the specified
                <i>key</i>.
      */
      std::shared_ptr<CallbackWriterQueue<Value>> GetSlot(Key key);

      virtual bool IsEmpty() const override;

      virtual Target Top() const override;

      virtual void Pop() override;

      virtual void Break(const std::exception_ptr& exception) override;

      using QueueReader<KeyValuePair<KeyType, ValueType>>::Break;
    protected:
      virtual bool IsAvailable() const override;

    private:
      Queue<Target> m_values;
      CallbackQueue m_callbacks;
  };

  template<typename KeyType, typename ValueType>
  TaggedQueue<KeyType, ValueType>::~TaggedQueue() {
    Break();
  }

  template<typename KeyType, typename ValueType>
  std::shared_ptr<CallbackWriterQueue<
      typename TaggedQueue<KeyType, ValueType>::Value>>
      TaggedQueue<KeyType, ValueType>::GetSlot(Key key) {
    return m_callbacks.GetSlot<Value>(
      [=] (const Value& value) {
        m_values.Push(MakeKeyValuePair(key, value));
      });
  }

  template<typename KeyType, typename ValueType>
  bool TaggedQueue<KeyType, ValueType>::IsEmpty() const {
    return m_values.IsEmpty();
  }

  template<typename KeyType, typename ValueType>
  typename TaggedQueue<KeyType, ValueType>::Target
      TaggedQueue<KeyType, ValueType>::Top() const {
    return m_values.Top();
  }

  template<typename KeyType, typename ValueType>
  void TaggedQueue<KeyType, ValueType>::Pop() {
    return m_values.Pop();
  }

  template<typename KeyType, typename ValueType>
  void TaggedQueue<KeyType, ValueType>::Break(
      const std::exception_ptr& exception) {
    m_callbacks.Break(exception);
    m_values.Break(exception);
  }

  template<typename KeyType, typename ValueType>
  bool TaggedQueue<KeyType, ValueType>::IsAvailable() const {
    return m_values.IsAvailable();
  }
}

#endif
