#ifndef BEAM_PUBLISHERREACTOR_HPP
#define BEAM_PUBLISHERREACTOR_HPP
#include <type_traits>
#include <utility>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Reactors/Event.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/SynchronizedList.hpp"

namespace Beam {
namespace Reactors {

  /*! \class PublisherReactor
      \brief A Reactor that evaluates to values produced by a Publisher.
      \tparam PublisherType The type Publisher to monitor.
   */
  template<typename PublisherType>
  class PublisherReactor : public Reactor<GetPublisherType<PublisherType>>,
      public Event {
    public:
      using Type = GetPublisherType<PublisherType>;

      //! Constructs a PublisherReactor.
      /*!
        \param publisher The Publisher to monitor.
      */
      template<typename PublisherForward>
      PublisherReactor(PublisherForward&& publisher);

      virtual void Commit();

      virtual Type Eval() const;

      virtual void Execute();

    private:
      GetOptionalLocalPtr<PublisherType> m_publisher;
      int m_state;
      Expect<Type> m_value;
      std::deque<Type> m_pendingValues;
      bool m_isBroken;
      SynchronizedVector<Type> m_updates;
      Threading::Sync<bool> m_pendingBreak;
      RoutineTaskQueue m_tasks;

      void OnUpdate(const Type& value);
      void OnBreak(const std::exception_ptr& exception);
      void S0();
      void S1();
      void S2();
      void S3();
  };

  //! Makes a PublisherReactor.
  /*!
    \param publisher The Publisher to monitor.
  */
  template<typename Publisher>
  std::shared_ptr<PublisherReactor<typename std::decay<Publisher>::type>>
      MakePublisherReactor(Publisher&& publisher) {
    return std::make_shared<PublisherReactor<
      typename std::decay<Publisher>::type>>(
      std::forward<Publisher>(publisher));
  }

  template<typename PublisherType>
  template<typename PublisherForward>
  PublisherReactor<PublisherType>::PublisherReactor(
      PublisherForward&& publisher)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_publisher(std::forward<PublisherForward>(publisher)),
        m_state(0) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::Commit() {
    m_updates.With(
      [&] (std::vector<Type>& updates) {
        for(auto&& update : updates) {
          m_pendingValues.push_back(std::move(update));
        }
        updates.clear();
      });
    m_isBroken = m_pendingBreak.Acquire();
    if(m_state == 0) {
      return S0();
    } else if(m_state == 1) {
      return S1();
    } else if(m_state == 2) {
      return S1();
    }
  }

  template<typename PublisherType>
  typename PublisherReactor<PublisherType>::Type
      PublisherReactor<PublisherType>::Eval() const {
    return m_value.Get();
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::Execute() {
    this->SignalUpdate();
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::OnUpdate(const Type& value) {
    m_updates.PushBack(value);
    this->SignalEvent();
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::OnBreak(
      const std::exception_ptr& exception) {
    m_pendingBreak = true;
    this->SignalEvent();
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::S0() {
    m_state = 0;
    m_isBroken = false;
    m_publisher->Monitor(m_tasks.GetSlot<Type>(
      std::bind(&PublisherReactor::OnUpdate, this, std::placeholders::_1),
      std::bind(&PublisherReactor::OnBreak, this, std::placeholders::_1)));
    return S1();
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::S1() {
    m_state = 1;
    if(!m_pendingValues.empty()) {

      // C0
      return S2();
    } else if(m_pendingValues.empty() && m_isBroken) {

      // ~C0 && C1
      return S3();
    }
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::S2() {
    m_state = 2;
    m_value = std::move(m_pendingValues.front());
    m_pendingValues.pop_front();
    this->IncrementSequenceNumber();
    if(m_pendingValues.empty() && m_isBroken) {

      // ~C0 && C1
      return S3();
    }
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::S3() {
    m_state = 3;
    this->SetComplete();
  }
}
}

#endif
