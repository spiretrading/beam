#ifndef BEAM_ALIASQUEUE_HPP
#define BEAM_ALIASQUEUE_HPP
#include <memory>
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /*! \class AliasQueue
      \brief Wraps a Queue whose lifetime is tied to another object.
      \tparam T The data to write to the Queue.
   */
  template<typename T>
  class AliasQueue : public QueueWriter<T> {
    private:
      struct Guard {};

    public:
      using Source = T;

      //! Constructs an AliasQueue, for internal use only.
      /*!
        \param queue The Queue to wrap.
        \param alias The object whose lifetime is managing <i>queue</i>.
      */
      AliasQueue(std::shared_ptr<QueueWriter<T>> queue,
        std::shared_ptr<void> alias, Guard);

      virtual ~AliasQueue() override;

      virtual void Push(const Source& value) override;

      virtual void Push(Source&& value) override;

      virtual void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;
    private:
      template<typename U> friend std::shared_ptr<AliasQueue<U>> MakeAliasQueue(
        std::shared_ptr<QueueWriter<U>> queue, std::shared_ptr<void> alias);
      std::shared_ptr<void> m_self;
      std::shared_ptr<QueueWriter<T>> m_queue;
      std::weak_ptr<void> m_alias;

      void Bind(std::shared_ptr<void> self);
  };

  //! Makes an AliasQueue.
  /*!
    \param queue The Queue to wrap.
    \param alias The object whose lifetime is managing <i>queue</i>.
    \return An AliasQueue wrapping the <i>queue</i>.
  */
  template<typename T>
  std::shared_ptr<AliasQueue<T>> MakeAliasQueue(
      std::shared_ptr<QueueWriter<T>> queue, std::shared_ptr<void> alias) {
    auto aliasQueue = std::make_shared<AliasQueue<T>>(std::move(queue),
      std::move(alias), typename AliasQueue<T>::Guard{});
    aliasQueue->Bind(aliasQueue);
    return aliasQueue;
  }

  template<typename T>
  AliasQueue<T>::AliasQueue(std::shared_ptr<QueueWriter<T>> queue,
      std::shared_ptr<void> alias, Guard)
      : m_queue{std::move(queue)},
        m_alias{std::move(alias)} {}

  template<typename T>
  AliasQueue<T>::~AliasQueue() {
    Break();
  }

  template<typename T>
  void AliasQueue<T>::Push(const Source& value) {
    auto alias = m_alias.lock();
    if(alias == nullptr) {
      m_queue.reset();
      m_self.reset();
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    m_queue->Push(value);
  }

  template<typename T>
  void AliasQueue<T>::Push(Source&& value) {
    auto alias = m_alias.lock();
    if(alias == nullptr) {
      m_queue.reset();
      m_self.reset();
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    m_queue->Push(std::move(value));
  }

  template<typename T>
  void AliasQueue<T>::Break(const std::exception_ptr& e) {
    auto alias = m_alias.lock();
    if(alias == nullptr) {
      m_queue.reset();
      m_self.reset();
      return;
    }
    m_queue->Break(e);
  }

  template<typename T>
  void AliasQueue<T>::Bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
  }
}

#endif
