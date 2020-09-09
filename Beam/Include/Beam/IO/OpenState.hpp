#ifndef BEAM_OPEN_STATE_HPP
#define BEAM_OPEN_STATE_HPP
#include <atomic>
#include <cassert>
#include <cstdint>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/Threading/ConditionVariable.hpp"

namespace Beam::IO {

  /** Stores state about whether a Connection/Client is open. */
  class OpenState {
    public:

      /** Constructs an OpenState. */
      OpenState();

      ~OpenState();

      /** Returns <code>true</code> iff the state is open. */
      bool IsOpen() const;

      /** Returns <code>true</code> iff the state is closing. */
      bool IsClosing() const;

      /** Returns <code>true</code> iff the state is closed. */
      bool IsClosed() const;

      /** Sets the state to closing and returns the prior closing state. */
      bool SetClosing();

      /** Sets the state to closed and returns the prior open state. */
      void Close();

    private:
      enum class State : std::uint8_t {
        OPEN,
        CLOSING,
        CLOSED
      };
      mutable boost::mutex m_mutex;
      std::atomic<State> m_state;
      Threading::ConditionVariable m_closingCondition;

      OpenState(const OpenState&) = delete;
      OpenState& operator =(const OpenState&) = delete;
  };

  inline OpenState::OpenState()
    : m_state(State::OPEN) {}

  inline OpenState::~OpenState() {
    assert(IsClosed());
  }

  inline bool OpenState::IsOpen() const {
    return m_state == State::OPEN;
  }

  inline bool OpenState::IsClosing() const {
    return m_state == State::CLOSING;
  }

  inline bool OpenState::IsClosed() const {
    return m_state == State::CLOSED;
  }

  inline bool OpenState::SetClosing() {
    static auto OPEN = State::OPEN;
    if(m_state.compare_exchange_strong(OPEN, State::CLOSING)) {
      return false;
    }
    if(m_state == State::CLOSED) {
      return true;
    }
    auto lock = boost::unique_lock(m_mutex);
    while(m_state != State::CLOSED) {
      m_closingCondition.wait(lock);
    }
    return true;
  }

  inline void OpenState::Close() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_state.exchange(State::CLOSED) != State::CLOSED) {
      m_closingCondition.notify_all();
    }
  }
}

#endif
