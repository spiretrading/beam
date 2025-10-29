#ifndef BEAM_STATE_QUEUE_HPP
#define BEAM_STATE_QUEUE_HPP
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Threading/ConditionVariable.hpp"

namespace Beam {

  /**
   * Stores only the most recent value pushed onto it.
   * @tparam T The data to store.
   */
  template<typename T>
  class StateQueue : public AbstractQueue<T> {
    public:
      using Target = typename AbstractQueue<T>::Target;
      using Source = typename AbstractQueue<T>::Source;

      /** Constructs a StateQueue. */
      StateQueue() = default;

      /** Blocks until a value is available and returns it without popping. */
      Source peek() const;

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& exception) override;
      using AbstractQueue<T>::close;

    private:
      mutable boost::mutex m_mutex;
      mutable ConditionVariable m_is_available_condition;
      boost::optional<Target> m_value;
      std::exception_ptr m_break_exception;

      bool unlocked_is_available() const;
  };

  template<typename T>
  typename StateQueue<T>::Source StateQueue<T>::peek() const {
    auto lock = boost::unique_lock(m_mutex);
    while(!unlocked_is_available()) {
      m_is_available_condition.wait(lock);
    }
    if(!m_value) {
      std::rethrow_exception(m_break_exception);
    }
    return *m_value;
  }

  template<typename T>
  typename StateQueue<T>::Source StateQueue<T>::pop() {
    auto lock = boost::unique_lock(m_mutex);
    while(!unlocked_is_available()) {
      m_is_available_condition.wait(lock);
    }
    if(!m_value) {
      std::rethrow_exception(m_break_exception);
    }
    auto value = std::move(*m_value);
    m_value = boost::none;
    return value;
  }

  template<typename T>
  boost::optional<typename StateQueue<T>::Source> StateQueue<T>::try_pop() {
    auto lock = boost::lock_guard(m_mutex);
    if(!m_value) {
      return boost::none;
    }
    auto value = std::move(*m_value);
    m_value = boost::none;
    return value;
  }

  template<typename T>
  void StateQueue<T>::push(const Target& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_break_exception) {
      std::rethrow_exception(m_break_exception);
    }
    if(m_value) {
      *m_value = value;
    } else {
      m_value.emplace(value);
      m_is_available_condition.notify_all();
    }
  }

  template<typename T>
  void StateQueue<T>::push(Target&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_break_exception) {
      std::rethrow_exception(m_break_exception);
    }
    if(m_value) {
      *m_value = std::move(value);
    } else {
      m_value.emplace(std::move(value));
      m_is_available_condition.notify_all();
    }
  }

  template<typename T>
  void StateQueue<T>::close(const std::exception_ptr& exception) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_break_exception) {
      return;
    }
    m_break_exception = exception;
    m_is_available_condition.notify_all();
  }

  template<typename T>
  bool StateQueue<T>::unlocked_is_available() const {
    return m_value || m_break_exception;
  }
}

#endif
