#ifndef BEAM_FILTERED_PUBLISHER_HPP
#define BEAM_FILTERED_PUBLISHER_HPP
#include <functional>
#include <unordered_set>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
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

  /**
   * Filters values that get sent to a Publisher.
   * @param <P> The type of Publisher to filter.
   */
  template<typename P>
  class FilteredPublisher final : public Details::GetPublisherType<P>::type,
      public QueueWriter<typename P::Type> {
    public:
      using Type = typename P::Type;
      using Snapshot = typename Details::GetSnapshotType<P>::type;

      /**
       * Defines the function used to filter values.
       * @param value The value to test.
       * @return <code>true</code> iff the <i>value</i> is to be published.
       */
      using FilterFunction = std::function<bool(const Type& value)>;

      /**
       * Constructs a FilteredPublisher.
       * @param filter The filter to apply.
       * @param publisher Initializes the Publisher to filter values for.
       */
      template<typename PF>
      FilteredPublisher(const FilterFunction& filter, PF&& publisher);

      void WithSnapshot(const std::function<
        void (boost::optional<const Snapshot&>)>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const override;

      void With(const std::function<void ()>& f) const override;

      void Monitor(std::shared_ptr<QueueWriter<Type>> monitor) const override;

      void Push(const Type& value) override;

      void Push(Type&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<Type>::Break;
    private:
      mutable boost::mutex m_mutex;
      FilterFunction m_filter;
      LocalPtr<P> m_publisher;
  };

  template<typename P>
  template<typename PF>
  FilteredPublisher<P>::FilteredPublisher(const FilterFunction& filter,
    PF&& publisher)
    : m_filter(filter),
      m_publisher(std::forward<PF>(publisher)) {}

  template<typename P>
  void FilteredPublisher<P>::WithSnapshot(
      const std::function<void (boost::optional<const Snapshot&>)>& f) const {
    m_publisher->WithSnapshot(f);
  }

  template<typename P>
  void FilteredPublisher<P>::Monitor(std::shared_ptr<QueueWriter<Type>> monitor,
      Out<boost::optional<Snapshot>> snapshot) const {
    m_publisher->Monitor(monitor, Store(snapshot));
  }

  template<typename P>
  void FilteredPublisher<P>::With(const std::function<void ()>& f) const {
    m_publisher->With(f);
  }

  template<typename P>
  void FilteredPublisher<P>::Monitor(
      std::shared_ptr<QueueWriter<Type>> queue) const {
    m_publisher->Monitor(queue);
  }

  template<typename P>
  void FilteredPublisher<P>::Push(const Type& value) {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(!m_filter(value)) {
        return;
      }
    }
    m_publisher->Push(value);
  }

  template<typename P>
  void FilteredPublisher<P>::Push(Type&& value) {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(!m_filter(value)) {
        return;
      }
    }
    m_publisher->Push(std::move(value));
  }

  template<typename P>
  void FilteredPublisher<P>::Break(const std::exception_ptr& e) {
    m_publisher->Break(e);
  }
}

#endif
