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
      using Source = typename QueueWriter<T>::Source;

      /**
       * Constructs an AliasQueueWriter, for internal use only.
       * @param queue The QueueWriter to wrap.
       * @param alias The object whose lifetime is managing <i>queue</i>.
       */
      AliasQueueWriter(ScopedQueueWriter<Source> queue,
        std::shared_ptr<void> alias, Guard);

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      template<typename QueueWriter>
      friend auto MakeAliasQueueWriter(QueueWriter&&, std::shared_ptr<void>);
      mutable boost::mutex m_mutex;
      std::shared_ptr<void> m_self;
      ScopedQueueWriter<Source> m_queue;
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
    using Source = typename GetTryDereferenceType<QueueWriter>::Source;
    auto aliasQueue = std::make_shared<AliasQueueWriter<Source>>(
      std::forward<QueueWriter>(queue), std::move(alias),
      typename AliasQueueWriter<Source>::Guard());
    aliasQueue->Bind(aliasQueue);
    return aliasQueue;
  }

  template<typename T>
  AliasQueueWriter<T>::AliasQueueWriter(ScopedQueueWriter<Source> queue,
    std::shared_ptr<void> alias, Guard)
    : m_queue(std::move(queue)),
      m_alias(std::move(alias)) {}

  template<typename T>
  void AliasQueueWriter<T>::Push(const Source& value) {
    auto alias = GetAlias();
    m_queue.Push(value);
  }

  template<typename T>
  void AliasQueueWriter<T>::Push(Source&& value) {
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
