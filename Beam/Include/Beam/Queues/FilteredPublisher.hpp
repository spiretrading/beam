#ifndef BEAM_FILTEREDPUBLISHER_HPP
#define BEAM_FILTEREDPUBLISHER_HPP
#include <functional>
#include <unordered_set>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"

namespace Beam {
  template<typename T>
  struct UniqueFilter {
    std::unordered_set<T> m_values;

    bool operator ()(const T& value) {
      return m_values.insert(value).second;
    }
  };

  /*! \class FilteredPublisher
      \brief Filters values that get sent to a Publisher.
      \tparam PublisherType The type of Publisher to filter.
   */
  template<typename PublisherType>
  class FilteredPublisher :
      public Details::GetPublisherType<PublisherType>::type,
      public QueueWriter<typename PublisherType::Type> {
    public:
      typedef typename PublisherType::Type Type;
      typedef typename Details::GetSnapshotType<PublisherType>::type Snapshot;

      //! Defines the function used to filter values.
      /*!
        \param value The value to test.
        \return <code>true</code> iff the <i>value</i> is to be published.
      */
      typedef std::function<bool (const Type& value)> FilterFunction;

      //! Constructs a FilteredPublisher.
      /*!
        param filter The filter to apply.
        \param publisher Initializes the Publisher to filter values for.
      */
      template<typename PublisherForward>
      FilteredPublisher(const FilterFunction& filter,
        PublisherForward&& publisher);

      virtual ~FilteredPublisher() override final;

      virtual void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override final;

      virtual void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override final;

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Type>> monitor) const override final;

      virtual void Push(const Type& value) final;

      virtual void Push(Type&& value) final;

      virtual void Break(const std::exception_ptr& e) final;

      using QueueWriter<typename PublisherType::Type>::Break;
    private:
      mutable boost::mutex m_mutex;
      FilterFunction m_filter;
      LocalPtr<PublisherType> m_publisher;
  };

  template<typename PublisherType>
  template<typename PublisherForward>
  FilteredPublisher<PublisherType>::FilteredPublisher(
      const FilterFunction& filter, PublisherForward&& publisher)
      : m_filter(filter),
        m_publisher(std::forward<PublisherForward>(publisher)) {}

  template<typename PublisherType>
  FilteredPublisher<PublisherType>::~FilteredPublisher() {
    Break();
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    m_publisher->WithSnapshot(f);
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    m_publisher->Monitor(monitor, Store(snapshot));
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::With(
      const std::function<void ()>& f) const {
    m_publisher->With(f);
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    m_publisher->Monitor(queue);
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::Push(const Type& value) {
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      if(!m_filter(value)) {
        return;
      }
    }
    m_publisher->Push(value);
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::Push(Type&& value) {
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      if(!m_filter(value)) {
        return;
      }
    }
    m_publisher->Push(std::move(value));
  }

  template<typename PublisherType>
  void FilteredPublisher<PublisherType>::Break(const std::exception_ptr& e) {
    m_publisher->Break(e);
  }
}

#endif
