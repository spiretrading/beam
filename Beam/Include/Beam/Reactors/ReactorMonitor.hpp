#ifndef BEAM_REACTORMONITOR_HPP
#define BEAM_REACTORMONITOR_HPP
#include <boost/noncopyable.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ReactorMonitor
      \brief Monitors and updates a Reactor.
   */
  class ReactorMonitor : private boost::noncopyable {
    public:

      //! Constructs a ReactorMonitor.
      /*!
        \param The Reactor to monitor.
      */
      ReactorMonitor(std::shared_ptr<BaseReactor> reactor);

      ~ReactorMonitor();

      //! Waits for the Reactor to complete.
      void Wait() const;

      void Open();

      void Close();

    private:
      mutable Threading::Mutex m_mutex;
      std::shared_ptr<BaseReactor> m_reactor;
      bool m_isRunning;
      boost::signals2::scoped_connection m_connection;
      mutable Threading::ConditionVariable m_waitCondition;
      IO::OpenState m_openState;

      void OnUpdate();
      void Shutdown();
  };

  inline ReactorMonitor::ReactorMonitor(std::shared_ptr<BaseReactor> reactor)
      : m_reactor{std::move(reactor)},
        m_isRunning{false} {}

  inline ReactorMonitor::~ReactorMonitor() {
    Close();
  }

  inline void ReactorMonitor::Wait() const {
    boost::unique_lock<Threading::Mutex> lock{m_mutex};
    while(m_isRunning) {
      m_waitCondition.wait(lock);
    }
  }

  inline void ReactorMonitor::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_connection = m_reactor->ConnectUpdateSignal(
      std::bind(&ReactorMonitor::OnUpdate, this));
    m_isRunning = true;
    m_openState.SetOpen();
  }

  inline void ReactorMonitor::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void ReactorMonitor::Shutdown() {
    m_isRunning = false;
    m_waitCondition.notify_all();
    m_openState.SetClosed();
  }

  inline void ReactorMonitor::OnUpdate() {
    m_reactor->Commit();
    if(m_reactor->IsComplete()) {
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      m_isRunning = false;
    }
  }
}
}

#endif
