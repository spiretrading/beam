#ifndef BEAM_OPEN_STATE_HPP
#define BEAM_OPEN_STATE_HPP
#include <atomic>
#include <cassert>
#include <cstdint>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Threading/ConditionVariable.hpp"

namespace Beam {

  /** Stores state about whether a Connection/Client is open. */
  class OpenState {
    public:

      /** Constructs an OpenState. */
      OpenState() noexcept;

#ifndef NDEBUG
      ~OpenState();
#endif

      /** Returns <code>true</code> iff the state is open. */
      bool is_open() const;

      /** Returns <code>true</code> iff the state is closing. */
      bool is_closing() const;

      /** Returns <code>true</code> iff the state is closed. */
      bool is_closed() const;

      /**
       * Tests if the state is open and raises a NotConnectionException
       * otherwise.
       */
      void ensure_open() const;

      /** Sets the state to closing and returns the prior closing state. */
      bool set_closing();

      /** Sets the state to closed and returns the prior open state. */
      void close();

    private:
      enum class State : std::uint8_t {
        OPEN,
        CLOSING,
        CLOSED
      };
      mutable boost::mutex m_mutex;
      std::atomic<State> m_state;
      ConditionVariable m_closing_condition;

      OpenState(const OpenState&) = delete;
      OpenState& operator =(const OpenState&) = delete;
  };

  inline OpenState::OpenState() noexcept
    : m_state(State::OPEN) {}

#ifndef NDEBUG
  inline OpenState::~OpenState() {
    assert(is_closed());
  }
#endif

  inline bool OpenState::is_open() const {
    return m_state == State::OPEN;
  }

  inline bool OpenState::is_closing() const {
    return m_state == State::CLOSING;
  }

  inline bool OpenState::is_closed() const {
    return m_state == State::CLOSED;
  }

  inline void OpenState::ensure_open() const {
    if(m_state != State::OPEN) {
      boost::throw_with_location(EndOfFileException());
    }
  }

  inline bool OpenState::set_closing() {
    auto expected = State::OPEN;
    if(m_state.compare_exchange_strong(expected, State::CLOSING)) {
      return false;
    }
    if(expected == State::CLOSED) {
      return true;
    }
    auto lock = boost::unique_lock(m_mutex);
    while(m_state != State::CLOSED) {
      m_closing_condition.wait(lock);
    }
    return true;
  }

  inline void OpenState::close() {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(m_state.exchange(State::CLOSED) == State::CLOSED) {
        return;
      }
    }
    m_closing_condition.notify_all();
  }
}

#endif
