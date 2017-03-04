#ifndef BEAM_OPENSTATE_HPP
#define BEAM_OPENSTATE_HPP
#include <cassert>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Utilities/Rethrow.hpp"

namespace Beam {
namespace IO {

  /*! \class OpenState
      \brief Stores state about whether a Connection/Client is open.
   */
  class OpenState : private boost::noncopyable {
    public:

      //! Constructs an OpenState.
      OpenState();

      //! Constructs an OpenState.
      /*!
        \param isOpen Whether the initial state is open.
      */
      explicit OpenState(bool isOpen);

      ~OpenState();

      //! Returns <code>true</code> iff the state is opening.
      bool IsOpening() const;

      //! Returns <code>true</code> iff the state is open.
      bool IsOpen() const;

      //! Returns <code>true</code> iff the state is either opening or open.
      bool IsRunning() const;

      //! Returns <code>true</code> iff the state is closing.
      bool IsClosing() const;

      //! Returns <code>true</code> iff the state is closed.
      bool IsClosed() const;

      //! Sets the state to opening and returns the prior opening state.
      bool SetOpening();

      //! Sets the state to open and returns the prior open state.
      void SetOpen();

      //! Indicates that the Open operation failed, can only be invoked from
      //! within the body of a catch handler.
      void SetOpenFailure();

      //! Indicates that the Open operation failed.
      /*!
        \param exception The reason for the failure.
      */
      void SetOpenFailure(const std::exception_ptr& exception);

      //! Indicates that the Open operation failed.
      /*!
        \param exception The reason for the failure.
      */
      template<typename E>
      void SetOpenFailure(const E& exception);

      //! Sets the state to closing and returns the prior closing state.
      bool SetClosing();

      //! Sets the state to closed and returns the prior open state.
      void SetClosed();

    private:
      mutable boost::mutex m_mutex;
      bool m_isOpen;
      bool m_isTransitioning;
      Threading::ConditionVariable m_isTransitioningCondition;
      std::exception_ptr m_exception;

      bool LockedIsOpening() const;
      bool LockedIsOpen() const;
      bool LockedIsClosing() const;
      bool LockedIsClosed() const;
  };

  inline OpenState::OpenState()
      : OpenState{false} {}

  inline OpenState::OpenState(bool isOpen)
      : m_isOpen{isOpen},
        m_isTransitioning{false} {}

  inline OpenState::~OpenState() {
    assert(IsClosed());
  }

  inline bool OpenState::IsOpening() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return LockedIsOpening();
  }

  inline bool OpenState::IsOpen() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return LockedIsOpen();
  }

  inline bool OpenState::IsRunning() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return LockedIsOpening() || LockedIsOpen();
  }

  inline bool OpenState::IsClosing() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return LockedIsClosing();
  }

  inline bool OpenState::IsClosed() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return LockedIsClosed();
  }

  inline bool OpenState::SetOpening() {
    boost::unique_lock<boost::mutex> lock{m_mutex};
    if(LockedIsOpen()) {
      return true;
    } else if(LockedIsOpening()) {
      while(LockedIsOpening()) {
        m_isTransitioningCondition.wait(lock);
      }
      Rethrow(m_exception);
      return true;
    } else if(LockedIsClosing()) {
      BOOST_THROW_EXCEPTION(ConnectException{"Connection is closing."});
    } else {
      assert(LockedIsClosed());
      m_isTransitioning = true;
      return false;
    }
  }

  inline void OpenState::SetOpenFailure() {
    SetOpenFailure(std::current_exception());
  }

  inline void OpenState::SetOpenFailure(const std::exception_ptr& exception) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    m_exception = exception;
  }

  template<typename E>
  void OpenState::SetOpenFailure(const E& exception) {
    SetOpenFailure(std::make_exception_ptr(exception));
  }

  inline void OpenState::SetOpen() {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    if(LockedIsOpen()) {
      return;
    }
    m_isTransitioning = false;
    m_isOpen = true;
    m_isTransitioningCondition.notify_all();
  }

  inline bool OpenState::SetClosing() {
    boost::unique_lock<boost::mutex> lock{m_mutex};
    if(LockedIsOpen()) {
      m_isTransitioning = true;
      return false;
    } else if(LockedIsClosing()) {
      while(LockedIsClosing()) {
        m_isTransitioningCondition.wait(lock);
      }
      return true;
    } else {
      return true;
    }
  }

  inline void OpenState::SetClosed() {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    if(LockedIsClosed()) {
      return;
    }
    m_isTransitioning = false;
    m_isOpen = false;
    m_isTransitioningCondition.notify_all();
    auto exception = m_exception;
    m_exception = std::exception_ptr{};
    Rethrow(exception);
  }

  inline bool OpenState::LockedIsOpening() const {
    return m_isTransitioning && !m_isOpen;
  }

  inline bool OpenState::LockedIsOpen() const {
    return !m_isTransitioning && m_isOpen;
  }

  inline bool OpenState::LockedIsClosing() const {
    return m_isTransitioning && m_isOpen;
  }

  inline bool OpenState::LockedIsClosed() const {
    return !m_isTransitioning && !m_isOpen;
  }
}
}

#endif
