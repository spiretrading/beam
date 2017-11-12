#ifndef BEAM_BASIC_REACTOR_HPP
#define BEAM_BASIC_REACTOR_HPP
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class BasicReactor
      \brief A Reactor that programmatically updates.
   */
  template<typename T>
  class BasicReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a BasicReactor.
      BasicReactor();

      //! Updates this Reactor with a value.
      /*!
        \param value The value to update.
      */
      void Update(T value);

      //! Brings this Reactor to a completion state.
      void SetComplete();

      //! Brings this Reactor to a completion state by throwing an exception.
      /*!
        \param e The exception to throw.
      */
      void SetComplete(std::exception_ptr e);

      //! Brings this Reactor to a completion state by throwing an exception.
      /*!
        \param e The exception to throw.
      */
      template<typename E>
      void SetComplete(const E& e);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::shared_ptr<Queue<T>> m_queue;
      QueueReactor<T> m_reactor;
  };

  //! Makes a BasicReactor.
  template<typename T>
  auto MakeBasicReactor() {
    return std::make_shared<BasicReactor<T>>();
  }

  template<typename T>
  BasicReactor<T>::BasicReactor()
      : m_queue{std::make_shared<Queue<T>>()},
        m_reactor{m_queue} {}

  template<typename T>
  void BasicReactor<T>::Update(T value) {
    m_queue->Push(std::move(value));
  }

  template<typename T>
  void BasicReactor<T>::SetComplete() {
    m_queue->Break();
  }

  template<typename T>
  void BasicReactor<T>::SetComplete(std::exception_ptr e) {
    m_queue->Break(std::move(e));
  }

  template<typename T>
  template<typename E>
  void BasicReactor<T>::SetComplete(const E& e) {
    m_queue->Break(e);
  }

  template<typename T>
  BaseReactor::Update BasicReactor<T>::Commit(int sequenceNumber) {
    return m_reactor.Commit(sequenceNumber);
  }

  template<typename T>
  typename BasicReactor<T>::Type BasicReactor<T>::Eval() const {
    return m_reactor.Eval();
  }
}
}

#endif
