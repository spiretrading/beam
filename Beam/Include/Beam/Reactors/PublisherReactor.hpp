#ifndef BEAM_PUBLISHERREACTOR_HPP
#define BEAM_PUBLISHERREACTOR_HPP
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class PublisherReactor
      \brief A Reactor that evaluates to values produced by a Publisher.
      \tparam PublisherType The type Publisher to monitor.
   */
  template<typename PublisherType>
  class PublisherReactor : public Reactor<GetPublisherType<PublisherType>> {
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

    private:
      mutable boost::mutex m_mutex;
      GetOptionalLocalPtr<PublisherType> m_publisher;
      Expect<T> m_value;
      std::vector<T> m_pendingValues;
      std::exception_ptr m_exception;
      RoutineTaskQueue m_tasks;

      void OnUpdate(const Type& value);
      void OnBreak(const std::exception_ptr& exception);
  };

  //! Makes a PublisherReactor.
  /*!
    \param publisher The Publisher to monitor.
  */
  template<typename Publisher>
  auto MakePublisherReactor(Publisher&& publisher) {
    return std::make_shared<PublisherReactor<
      typename std::decay<Publisher>::type>>(
      std::forward<Publisher>(publisher));
  }

  template<typename PublisherType>
  template<typename PublisherForward>
  PublisherReactor<PublisherType>::PublisherReactor(
      PublisherForward&& publisher)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_publisher{std::forward<PublisherForward>(publisher)} {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::Commit() {}

  template<typename PublisherType>
  typename PublisherReactor<PublisherType>::Type
      PublisherReactor<PublisherType>::Eval() const {
    return m_value.Get();
  }

  template<typename PublisherType>
  void PublisherReactor<PublisherType>::OnUpdate(const Type& value) {
    auto signalUpdate = false;
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      m_pendingValues.push_back(value);
      if(m_pendingValues.size() == 1) {
        signalUpdate = true;
      }
    }
    if(signalUpdate) {
      this->SignalUpdate();
    }
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
