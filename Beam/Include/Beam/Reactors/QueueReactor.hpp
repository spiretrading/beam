#ifndef BEAM_QUEUE_REACTOR_HPP
#define BEAM_QUEUE_REACTOR_HPP
#include <memory>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/Trigger.hpp"

namespace Beam {
namespace Reactors {

  /*! \class QueueReactor
      \brief Evaluates to a constant.
   */
  template<typename T>
  class QueueReactor : public Reactor<T> {
    public:
      using Type = GetReactorType<Reactor<T>>;

      //! Constructs a QueueReactor.
      /*!
        \param queue The Queue to monitor.
        \param trigger The Trigger to signal when an update is available.
      */
      QueueReactor(std::shared_ptr<QueueReader<Type>> queue,
        RefType<Trigger> trigger);

      virtual bool IsComplete() const override;

      virtual void Commit(int sequenceNumber) override;

      virtual Type Eval() const override;

    private:
      std::shared_ptr<QueueReader<Type>> m_queue;
      Trigger* m_trigger;
      bool m_isComplete;
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
  QueueReactor<T>::QueueReactor(std::shared_ptr<QueueReader<Type>> queue,
      RefType<Trigger> trigger)
      : m_queue{std::move(queue)},
        m_trigger{trigger.Get()},
        m_isComplete{false} {}

  template<typename T>
  bool QueueReactor<T>::IsComplete() const {
    return m_isComplete;
  }

  template<typename T>
  void QueueReactor<T>::Commit(int sequenceNumber) {}

  template<typename T>
  typename QueueReactor<T>::Type QueueReactor<T>::Eval() const {
    throw NotImplementedException{};
  }
}
}

#endif
