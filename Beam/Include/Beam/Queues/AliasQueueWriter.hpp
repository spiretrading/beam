#ifndef BEAM_ALIAS_QUEUE_WRITER_HPP
#define BEAM_ALIAS_QUEUE_WRITER_HPP
#include <memory>
#include <utility>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /**
   * Wraps a QueueWriter whose lifetime is tied to another object.
   * @param <T> The data to write to the QueueWriter.
   */
  template<typename T>
  class AliasQueueWriter : public QueueWriter<T> {
    private:
      struct Guard {};

    public:
      using Target = typename QueueWriter<T>::Target;

      /**
       * Constructs an AliasQueueWriter, for internal use only.
       * @param queue The QueueWriter to wrap.
       * @param alias The object whose lifetime is managing <i>queue</i>.
       */
      AliasQueueWriter(ScopedQueueWriter<Target> queue,
        std::shared_ptr<void> alias, Guard);

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      template<typename QueueWriter>
      friend auto MakeAliasQueueWriter(QueueWriter&&, std::shared_ptr<void>);
      mutable boost::mutex m_mutex;
      std::shared_ptr<void> m_self;
      ScopedQueueWriter<Target> m_queue;
      std::weak_ptr<void> m_alias;

      std::shared_ptr<void> GetAlias();
      void Bind(std::shared_ptr<void> self);
  };

  /**
   * Makes an AliasQueueWriter.
   * @param queue The QueueWriter to wrap.
   * @param alias The object whose lifetime is managing <i>queue</i>.
   */
  template<typename QueueWriter>
  auto MakeAliasQueueWriter(QueueWriter&& queue, std::shared_ptr<void> alias) {
    using Target = typename GetTryDereferenceType<QueueWriter>::Target;
    auto aliasQueue = std::make_shared<AliasQueueWriter<Target>>(
      std::forward<QueueWriter>(queue), std::move(alias),
      typename AliasQueueWriter<Target>::Guard());
    aliasQueue->Bind(aliasQueue);
    return aliasQueue;
  }

  template<typename T>
  AliasQueueWriter<T>::AliasQueueWriter(ScopedQueueWriter<Target> queue,
    std::shared_ptr<void> alias, Guard)
    : m_queue(std::move(queue)),
      m_alias(std::move(alias)) {}

  template<typename T>
  void AliasQueueWriter<T>::Push(const Target& value) {
    auto alias = GetAlias();
    m_queue.Push(value);
  }

  template<typename T>
  void AliasQueueWriter<T>::Push(Target&& value) {
    auto alias = GetAlias();
    m_queue.Push(std::move(value));
  }

  template<typename T>
  void AliasQueueWriter<T>::Break(const std::exception_ptr& e) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(!m_self) {
        return;
      }
      m_alias.reset();
      m_self.reset();
    }
    m_queue.Break(e);
  }

  template<typename T>
  std::shared_ptr<void> AliasQueueWriter<T>::GetAlias() {
    {
      auto lock = std::lock_guard(m_mutex);
      if(!m_self) {
        return nullptr;
      }
      auto alias = m_alias.lock();
      if(alias) {
        return alias;
      }
      m_self.reset();
    }
    m_queue.Break();
    return nullptr;
  }

  template<typename T>
  void AliasQueueWriter<T>::Bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
  }
}

#endif
