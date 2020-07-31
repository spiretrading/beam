#ifndef BEAM_STATE_QUEUE_HPP
#define BEAM_STATE_QUEUE_HPP
#include <optional>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /**
   * Stores only the most recent value pushed onto it.
   * @param <T> The data to store.
   */
  template<typename T>
  class StateQueue : public AbstractQueue<T> {
    public:
      using Source = typename AbstractQueue<T>::Source;
      using Target = typename AbstractQueue<T>::Target;

      /** Constructs a StateQueue. */
      StateQueue() = default;

      bool IsEmpty() const override;

      Target Top() const override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      void Pop() override;

      /** For internal use by other Queues only. */
      bool IsAvailable() const;

      using QueueWriter<Source>::Break;
      using Threading::Waitable::Wait;

    private:
      std::optional<Source> m_value;
      std::exception_ptr m_breakException;
  };

  template<typename T>
  bool StateQueue<T>::IsEmpty() const {
    auto lock = boost::lock_guard(this->GetMutex());
    return !m_value.has_value();
  }

  template<typename T>
  typename StateQueue<T>::Target StateQueue<T>::Top() const {
    auto lock = boost::unique_lock(this->GetMutex());
    this->Wait(lock);
    if(!m_value) {
      std::rethrow_exception(m_breakException);
    }
    return *m_value;
  }

  template<typename T>
  void StateQueue<T>::Push(const Source& value) {
    auto lock = boost::lock_guard(this->GetMutex());
    if(m_breakException) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value) {
      *m_value = value;
    } else {
      m_value.emplace(value);
      this->NotifyOne();
    }
  }

  template<typename T>
  void StateQueue<T>::Push(Source&& value) {
    auto lock = boost::lock_guard(this->GetMutex());
    if(m_breakException) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value) {
      *m_value = std::move(value);
    } else {
      m_value.emplace(std::move(value));
      this->NotifyOne();
    }
  }

  template<typename T>
  void StateQueue<T>::Break(const std::exception_ptr& exception) {
    auto lock = boost::lock_guard(this->GetMutex());
    if(m_breakException) {
      return;
    }
    m_breakException = exception;
    this->NotifyAll();
  }

  template<typename T>
  void StateQueue<T>::Pop() {
    auto lock = boost::lock_guard(this->GetMutex());
    m_value = std::nullopt;
  }

  template<typename T>
  bool StateQueue<T>::IsAvailable() const {
    return m_value || m_breakException;
  }
}

#endif
