#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <deque>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class QueueReactor
      \brief Emits values published by a Queue.
   */
  template<typename T>
  class QueueReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a QueueReactor.
      /*!
        \param queue The Queue to monitor.
        \param trigger The Trigger to signal when an update is available.
      */
      QueueReactor(std::shared_ptr<QueueReader<Type>> queue,
        RefType<Trigger> trigger);

      ~QueueReactor();

      virtual bool IsInitialized() const override;

      virtual bool IsComplete() const override;

      virtual BaseReactor::Update Commit(int sequenceNumber) override;

      virtual Type Eval() const override;

    private:
      struct Update {
        Expect<Type> m_value;
        int m_sequenceNumber;

        Update(const Expect<Type>& value);
      };
      mutable boost::mutex m_mutex;
      std::shared_ptr<QueueReader<Type>> m_queue;
      Trigger* m_trigger;
      bool m_isComplete;
      bool m_failed;
      std::deque<Update> m_updates;
      Update m_value;
      Routines::RoutineHandler m_queueListener;

      void OnUpdate(const Type& value);
      void OnBreak(const std::exception_ptr& breakCondition);
  };

  //! Makes a QueueReactor.
  /*!
    \param queue The Queue to monitor.
    \param trigger The Trigger to signal when an update is available.
  */
  template<typename T>
  auto MakeQueueReactor(std::shared_ptr<QueueReader<T>> queue,
      RefType<Trigger> trigger) {
    return std::make_shared<QueueReactor<T>>(std::move(queue), Ref(trigger));
  }

  template<typename T>
  QueueReactor<T>::Update::Update(const Expect<Type>& value)
      : m_value{value},
        m_sequenceNumber{-1} {}

  template<typename T>
  QueueReactor<T>::QueueReactor(std::shared_ptr<QueueReader<Type>> queue,
      RefType<Trigger> trigger)
      : m_queue{std::move(queue)},
        m_trigger{trigger.Get()},
        m_isComplete{false},
        m_failed{false},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})} {
    m_queueListener = Routines::Spawn(
      [=] {
        Monitor(m_queue,
          std::bind(&QueueReactor::OnUpdate, this, std::placeholders::_1),
          std::bind(&QueueReactor::OnBreak, this, std::placeholders::_1));
      });
  }

  template<typename T>
  QueueReactor<T>::~QueueReactor() {
    m_queue->Break();
  }

  template<typename T>
  bool QueueReactor<T>::IsInitialized() const {
    return m_value.m_sequenceNumber != -1;
  }

  template<typename T>
  bool QueueReactor<T>::IsComplete() const {
    return m_isComplete;
  }

  template<typename T>
  BaseReactor::Update QueueReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber < m_value.m_sequenceNumber) {
      return BaseReactor::Update::NONE;
    } else if(sequenceNumber == m_value.m_sequenceNumber) {
      if(m_isComplete && !m_failed) {
        return BaseReactor::Update::COMPLETE;
      } else {
        return BaseReactor::Update::EVAL;
      }
    } else if(m_isComplete) {
      return BaseReactor::Update::NONE;
    }
    boost::lock_guard<boost::mutex> lock{m_mutex};
    while(!m_updates.empty() &&
        m_updates.front().m_sequenceNumber < sequenceNumber) {
      m_updates.pop_front();
    }
    if(m_updates.empty() ||
        m_updates.front().m_sequenceNumber > sequenceNumber) {
      return BaseReactor::Update::NONE;
    }
    if(m_updates.front().m_value.IsValue()) {
      m_value = std::move(m_updates.front());
      m_updates.pop_front();
      return BaseReactor::Update::EVAL;
    }
    m_value.m_sequenceNumber = sequenceNumber;
    m_isComplete = true;
    auto e = m_updates.front().m_value.GetException();
    m_updates = std::deque<Update>{};
    try {
      std::rethrow_exception(e);
    } catch(const PipeBrokenException&) {
      return BaseReactor::Update::COMPLETE;
    } catch(const std::exception&) {
      m_failed = true;
      m_value.m_value = e;
      return BaseReactor::Update::EVAL;
    }
  }

  template<typename T>
  typename QueueReactor<T>::Type QueueReactor<T>::Eval() const {
    return m_value.m_value.Get();
  }

  template<typename T>
  void QueueReactor<T>::OnUpdate(const Type& value) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    m_updates.emplace_back(value);
    m_trigger->SignalUpdate(Store(m_updates.back().m_sequenceNumber));
  }

  template<typename T>
  void QueueReactor<T>::OnBreak(const std::exception_ptr& breakCondition) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    m_updates.emplace_back(breakCondition);
    m_trigger->SignalUpdate(Store(m_updates.back().m_sequenceNumber));
  }
}
}

#endif
