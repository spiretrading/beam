#ifndef BEAM_FILTERED_QUEUE_WRITER_HPP
#define BEAM_FILTERED_QUEUE_WRITER_HPP
#include <mutex>
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /**
   * Filters data pushed onto a QueueWriter.
   * @param <T> The type of data to filter.
   * @param <F> The type used to filter the data.
   */
  template<typename T, typename F>
  class FilteredQueueWriter : public QueueWriter<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /**
       * The type used to filter the data.
       * @param value The value to test.
       * @return <code>false</code> iff the <i>value</i> should be filtered out.
       */
      using Filter = F;

      /**
       * Constructs a FilteredQueueWriter.
       * @param destination The QueueWriter to push the filtered data onto.
       * @param filter The function used to filter values.
       */
      template<typename FF>
      FilteredQueueWriter(ScopedQueueWriter<Target> destination, FF&& filter);

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      mutable std::mutex m_mutex;
      std::exception_ptr m_exception;
      ScopedQueueWriter<Target> m_destination;
      Filter m_filter;
  };

  template<typename QueueWriter, typename F>
  FilteredQueueWriter(QueueWriter&&, F&&) -> FilteredQueueWriter<
    typename GetTryDereferenceType<QueueWriter>::Target, std::decay_t<F>>;

  /**
   * Makes a FilteredWriterQueue.
   * @param destination The QueueWriter to push the filtered data onto.
   * @param filter The function performing the filtering.
   */
  template<typename QueueWriter, typename F>
  auto MakeFilteredQueueWriter(QueueWriter&& destination, F&& filter) {
    using Target = typename GetTryDereferenceType<QueueWriter>::Target;
    return std::make_shared<FilteredQueueWriter<Target, std::decay_t<F>>>(
      std::forward<QueueWriter>(destination), std::forward<F>(filter));
  }

  template<typename T, typename F>
  template<typename FF>
  FilteredQueueWriter<T, F>::FilteredQueueWriter(
    ScopedQueueWriter<Target> destination, FF&& filter)
    : m_destination(std::move(destination)),
      m_filter(std::forward<FF>(filter)) {}

  template<typename T, typename F>
  void FilteredQueueWriter<T, F>::Push(const Target& value) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        std::rethrow_exception(m_exception);
      }
    }
    if(m_filter(value)) {
      m_destination.Push(value);
    }
  }

  template<typename T, typename F>
  void FilteredQueueWriter<T, F>::Push(Target&& value) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        std::rethrow_exception(m_exception);
      }
    }
    if(m_filter(value)) {
      m_destination.Push(std::move(value));
    }
  }

  template<typename T, typename F>
  void FilteredQueueWriter<T, F>::Break(const std::exception_ptr& e) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        return;
      }
      m_exception = e;
    }
    m_destination.Break(e);
  }
}

#endif
