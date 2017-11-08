#ifndef BEAM_REACTOR_MONITOR_HPP
#define BEAM_REACTOR_MONITOR_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/Trigger.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ReactorMonitor
      \brief Used to synchronously monitor multiple Reactors.
   */
  class ReactorMonitor : private boost::noncopyable {
    public:

      //! Constructs a ReactorMonitor.
      ReactorMonitor() = default;

      ~ReactorMonitor();

      //! Returns the Trigger receiving Reactor updates.
      const Trigger& GetTrigger() const;

      //! Returns the Trigger receiving Reactor updates.
      Trigger& GetTrigger();

      //! Adds a Reactor to monitor.
      /*!
        \param reactor The Reactor to monitor.
      */
      void Add(std::shared_ptr<BaseReactor> reactor);

      //! Invokes a callback from within this monitor's event handler.
      /*!
        \param f The callback to invoke.
      */
      template<typename F>
      void Do(F&& f);

      void Open();

      void Close();

    private:
      mutable boost::mutex m_mutex;
      IO::OpenState m_openState;
      Trigger m_trigger;
      std::vector<std::shared_ptr<BaseReactor>> m_reactors;
      RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnSequenceNumber(int sequenceNumber);
  };

  inline ReactorMonitor::~ReactorMonitor() {
    Close();
  }

  inline const Trigger& ReactorMonitor::GetTrigger() const {
    return m_trigger;
  }

  inline Trigger& ReactorMonitor::GetTrigger() {
    return m_trigger;
  }

  inline void ReactorMonitor::Add(std::shared_ptr<BaseReactor> reactor) {
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      auto reactorIterator = std::find(m_reactors.begin(), m_reactors.end(),
        reactor);
      if(reactorIterator != m_reactors.end()) {
        return;
      }
      m_reactors.push_back(reactor);
    }
    if(m_openState.IsOpen()) {
      m_tasks.Push(std::bind(&ReactorMonitor::OnSequenceNumber, this, 0));
    }
  }

  template<typename F>
  void ReactorMonitor::Do(F&& f) {
    m_tasks.Push(std::move(f));
  }

  inline void ReactorMonitor::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_trigger.GetSequenceNumberPublisher().Monitor(m_tasks.GetSlot<int>(
      std::bind(&ReactorMonitor::OnSequenceNumber, this,
      std::placeholders::_1)));
    m_tasks.Push(std::bind(&ReactorMonitor::OnSequenceNumber, this, 0));
    m_openState.SetOpen();
  }

  inline void ReactorMonitor::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void ReactorMonitor::Shutdown() {
    m_tasks.Break();
    m_tasks.Wait();
    m_openState.SetClosed();
  }

  inline void ReactorMonitor::OnSequenceNumber(int sequenceNumber) {
    std::vector<std::shared_ptr<BaseReactor>> reactors;
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      reactors = m_reactors;
    }
    for(auto& reactor : reactors) {
      reactor->Commit(sequenceNumber);
    }
  }
}
}

#endif
