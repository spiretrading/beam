#ifndef BEAM_REACTORMONITOR_HPP
#define BEAM_REACTORMONITOR_HPP
#include <algorithm>
#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/SignalHandling/ConnectionGroup.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ReactorMonitor
      \brief Monitors and updates Reactors.
   */
  class ReactorMonitor : private boost::noncopyable {
    public:

      //! Signals that a Reactor will no longer produce new values.
      using CompleteSignal = boost::signals2::signal<void ()>;

      //! Constructs a ReactorMonitor.
      ReactorMonitor() = default;

      ~ReactorMonitor();

      //! Adds an Event to monitor.
      /*!
        \param event The Event to monitor.
      */
      void AddEvent(std::shared_ptr<Event> event);

      //! Adds a Reactor to monitor.
      /*!
        \param reactor The Reactor to add.
      */
      void AddReactor(std::shared_ptr<BaseReactor> reactor);

      //! Connects a slot to a Reactor's CompleteSignal.
      /*!
        \param reactor The Reactor to connect the slot to.
        \param slot The slot to connect.
        \return A connection to the specified <i>slot</i>.
      */
      boost::signals2::connection ConnectCompleteSignal(
        const BaseReactor& reactor,
        const CompleteSignal::slot_type& slot) const;

      void Open();

      void Close();

    private:
      mutable Threading::RecursiveMutex m_mutex;
      std::vector<std::shared_ptr<Event>> m_events;
      std::vector<std::shared_ptr<BaseReactor>> m_reactors;
      IO::OpenState m_openState;
      SignalHandling::ConnectionGroup m_connections;
      std::vector<BaseReactor*> m_updatedReactors;
      std::vector<BaseReactor*> m_completedReactors;
      mutable std::unordered_map<const BaseReactor*, CompleteSignal>
        m_completeSignals;
      mutable RoutineTaskQueue m_tasks;

      void Shutdown();
      void Initialize(const std::shared_ptr<BaseReactor>& reactor);
      void OnEvent(const std::weak_ptr<Event>& event);
      void OnUpdate(const std::weak_ptr<BaseReactor>& reactor);
  };

  inline ReactorMonitor::~ReactorMonitor() {
    Close();
  }

  inline void ReactorMonitor::AddEvent(std::shared_ptr<Event> event) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    auto eventIterator = std::find(m_events.begin(), m_events.end(), event);
    if(eventIterator != m_events.end()) {
      return;
    }
    m_events.push_back(event);
    m_connections.AddConnection(event->ConnectEventSignal(
      std::bind(&ReactorMonitor::OnEvent, this, std::weak_ptr<Event>(event))));
  }

  inline void ReactorMonitor::AddReactor(std::shared_ptr<BaseReactor> reactor) {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    auto reactorIterator = std::find(m_reactors.begin(), m_reactors.end(),
      reactor);
    if(reactorIterator != m_reactors.end()) {
      return;
    }
    m_reactors.push_back(reactor);
    if(m_openState.IsOpen()) {
      Initialize(reactor);
    }
  }

  inline boost::signals2::connection ReactorMonitor::ConnectCompleteSignal(
      const BaseReactor& reactor, const CompleteSignal::slot_type& slot) const {
    boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
    if(reactor.IsComplete()) {
      auto delayedSlot = std::make_shared<CompleteSignal>();
      auto connection = delayedSlot->connect(slot);
      m_tasks.Push(
        [=] {
          (*delayedSlot)();
        });
      return connection;
    }
    return m_completeSignals[&reactor].connect(slot);
  }

  inline void ReactorMonitor::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    Routines::Async<void> openToken;
    m_tasks.Push(
      [&] {
        boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
        auto initializedReactors = std::size_t{0};
        while(initializedReactors != m_reactors.size()) {
          auto reactors = std::vector<std::shared_ptr<BaseReactor>>(
            m_reactors.begin() + initializedReactors, m_reactors.end());
          for(auto& reactor : reactors) {
            ++initializedReactors;
            Initialize(reactor);
          }
        }
        m_openState.SetOpen();
        openToken.GetEval().SetResult();
      });
    openToken.Get();
  }

  inline void ReactorMonitor::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void ReactorMonitor::Shutdown() {
    Routines::Async<void> closeToken;
    m_tasks.Push(
      [&] {
        m_reactors.clear();
        m_events.clear();
        m_connections.DisconnectAll();
        closeToken.GetEval().SetResult();
      });
    closeToken.Get();
    m_openState.SetClosed();
  }

  inline void ReactorMonitor::Initialize(
      const std::shared_ptr<BaseReactor>& reactor) {
    m_connections.AddConnection(reactor->ConnectUpdateSignal(
      std::bind(&ReactorMonitor::OnUpdate, this,
      std::weak_ptr<BaseReactor>(reactor))));
    reactor->Commit();
    if(reactor->IsComplete()) {
      m_tasks.Push(
        [=] {
          auto signalIterator = m_completeSignals.find(reactor.get());
          if(signalIterator != m_completeSignals.end()) {
            signalIterator->second();
          }
        });
    }
  }

  inline void ReactorMonitor::OnUpdate(
      const std::weak_ptr<BaseReactor>& weakReactor) {
    auto reactor = weakReactor.lock();
    if(reactor == nullptr) {
      return;
    }
    m_updatedReactors.push_back(reactor.get());
  }

  inline void ReactorMonitor::OnEvent(const std::weak_ptr<Event>& weakEvent) {
    m_tasks.Push(
      [=] {
        auto event = weakEvent.lock();
        if(event == nullptr) {
          return;
        }
        boost::lock_guard<Threading::RecursiveMutex> lock(m_mutex);
        event->Execute();
        for(auto& reactor : m_updatedReactors) {
          reactor->Commit();
          if(reactor->IsComplete()) {
            m_completedReactors.push_back(reactor);
          }
        }
        for(auto& reactor : m_completedReactors) {
          auto signalIterator = m_completeSignals.find(reactor);
          if(signalIterator != m_completeSignals.end()) {
            signalIterator->second();
          }
        }
        m_completedReactors.clear();
        m_updatedReactors.clear();
      });
  }
}
}

#endif
