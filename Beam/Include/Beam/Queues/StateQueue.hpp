#ifndef BEAM_STATE_QUEUE_HPP
#define BEAM_STATE_QUEUE_HPP
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Threading/ConditionVariable.hpp"

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

      Target Top() const override;

      boost::optional<Target> TryTop() const override;

      Target Pop() override;

      boost::optional<Target> TryPop() override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& exception) override;

      using AbstractQueue<T>::Break;

    private:
      mutable boost::mutex m_mutex;
      mutable Threading::ConditionVariable m_isAvailableCondition;
      boost::optional<Source> m_value;
      std::exception_ptr m_breakException;

      bool UnlockedIsAvailable() const;
  };

  template<typename T>
  typename StateQueue<T>::Target StateQueue<T>::Top() const {
    auto lock = boost::unique_lock(m_mutex);
    while(!UnlockedIsAvailable()) {
      m_isAvailableCondition.wait(lock);
    }
    if(!m_value) {
      std::rethrow_exception(m_breakException);
    }
    return *m_value;
  }

  template<typename T>
  boost::optional<typename StateQueue<T>::Target>
      StateQueue<T>::TryTop() const {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_value) {
      return boost::none;
    }
    return *m_value;
  }

  template<typename T>
  typename StateQueue<T>::Target StateQueue<T>::Pop() {
    auto lock = boost::unique_lock(m_mutex);
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
  boost::optional<typename StateQueue<T>::Target> StateQueue<T>::TryPop() {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_value) {
      return boost::none;
    }
    auto value = std::move(*m_value);
    m_value = boost::none;
    return value;
  }

  template<typename T>
  void StateQueue<T>::Push(const Source& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_breakException) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value) {
      *m_value = value;
    } else {
      m_value.emplace(value);
      m_isAvailableCondition.notify_one();
    }
  }

  template<typename T>
  void StateQueue<T>::Push(Source&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_breakException) {
      std::rethrow_exception(m_breakException);
    }
    if(m_value) {
      *m_value = std::move(value);
    } else {
      m_value.emplace(std::move(value));
      m_isAvailableCondition.notify_one();
    }
  }

  template<typename T>
  void StateQueue<T>::Break(const std::exception_ptr& exception) {
    auto lock = boost::lock_guard(m_mutex);
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
