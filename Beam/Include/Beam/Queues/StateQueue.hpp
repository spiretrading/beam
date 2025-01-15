#ifndef BEAM_STATE_QUEUE_HPP
#define BEAM_STATE_QUEUE_HPP
#include <condition_variable>
#include <mutex>
#include <boost/optional/optional.hpp>
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
      using Target = typename AbstractQueue<T>::Target;
      using Source = typename AbstractQueue<T>::Source;

      /** Constructs a StateQueue. */
      StateQueue() = default;

      /** Blocks until a value is available and returns it without popping. */
      Source Peek() const;

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using AbstractQueue<T>::Break;

    private:
      mutable std::mutex m_mutex;
      mutable std::condition_variable m_isAvailableCondition;
      boost::optional<Target> m_value;
      std::exception_ptr m_breakException;

      bool UnlockedIsAvailable() const;
  };

  template<typename T>
  typename StateQueue<T>::Source StateQueue<T>::Peek() const {
    auto lock = std::unique_lock(m_mutex);
    while(!UnlockedIsAvailable()) {
      m_isAvailableCondition.wait(lock);
    }
    if(!m_value) {
      std::rethrow_exception(m_breakException);
    }
    return *m_value;
  }

  template<typename T>
  typename StateQueue<T>::Source StateQueue<T>::Pop() {
    auto lock = std::unique_lock(m_mutex);
    while(!UnlockedIsAvailable()) {
      m_isAvailableCondition.wait(lock);
    }
    if(!m_value) {
      std::rethrow_exception(m_breakException);
    }
    auto value = std::move(*m_value);
    m_value = boost::none;
    return value;
  }

  template<typename T>
  boost::optional<typename StateQueue<T>::Source> StateQueue<T>::TryPop() {
    auto lock = std::lock_guard(m_mutex);
    if(!m_value) {
      return boost::none;
    }
    auto value = std::move(*m_value);
    m_value = boost::none;
    return value;
  }

  template<typename T>
  void StateQueue<T>::Push(const Target& value) {
    auto lock = std::lock_guard(m_mutex);
    if(m_breakException) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value) {
      *m_value = value;
    } else {
      m_value.emplace(value);
      m_isAvailableCondition.notify_all();
    }
  }

  template<typename T>
  void StateQueue<T>::Push(Target&& value) {
    auto lock = std::lock_guard(m_mutex);
    if(m_breakException) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value) {
      *m_value = std::move(value);
    } else {
      m_value.emplace(std::move(value));
      m_isAvailableCondition.notify_all();
    }
  }

  template<typename T>
  void StateQueue<T>::Break(const std::exception_ptr& exception) {
    auto lock = std::lock_guard(m_mutex);
    if(m_breakException) {
      return;
    }
    m_breakException = exception;
    m_isAvailableCondition.notify_all();
  }

  template<typename T>
  bool StateQueue<T>::UnlockedIsAvailable() const {
    return m_value || m_breakException;
  }
}

#endif
