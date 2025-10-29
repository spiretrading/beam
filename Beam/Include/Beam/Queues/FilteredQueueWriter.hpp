#ifndef BEAM_FILTERED_QUEUE_WRITER_HPP
#define BEAM_FILTERED_QUEUE_WRITER_HPP
#include <concepts>
#include <mutex>
#include <type_traits>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Filters data pushed onto a QueueWriter.
   * @tparam T The type of data to filter.
   * @tparam F The predicate used to filter the data.
   */
  template<typename T, std::predicate<const T&> F>
  class FilteredQueueWriter : public QueueWriter<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /** The predicate used to filter the data. */
      using Filter = F;

      /**
       * Constructs a FilteredQueueWriter.
       * @param destination The QueueWriter to push the filtered data onto.
       * @param filter The function used to filter values.
       */
      template<Initializes<F> FF>
      FilteredQueueWriter(ScopedQueueWriter<Target> destination, FF&& filter);

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<T>::close;

    private:
      mutable std::mutex m_mutex;
      std::exception_ptr m_exception;
      ScopedQueueWriter<Target> m_destination;
      Filter m_filter;
  };

  template<typename Q, typename F>
  FilteredQueueWriter(Q&&, F&&) -> FilteredQueueWriter<
    typename dereference_t<Q>::Target, std::remove_cvref_t<F>>;

  /**
   * Makes a FilteredWriterQueue.
   * @param destination The QueueWriter to push the filtered data onto.
   * @param filter The function performing the filtering.
   */
  template<
    typename Q, std::predicate<const typename dereference_t<Q>::Target&> F>
  auto filter(Q&& destination, F&& filter) {
    using Target = typename dereference_t<Q>::Target;
    return std::make_shared<
      FilteredQueueWriter<Target, std::remove_cvref_t<F>>>(
        std::forward<Q>(destination), std::forward<F>(filter));
  }

  template<typename T, std::predicate<const T&> F>
  template<Initializes<F> FF>
  FilteredQueueWriter<T, F>::FilteredQueueWriter(
    ScopedQueueWriter<Target> destination, FF&& filter)
    : m_destination(std::move(destination)),
      m_filter(std::forward<FF>(filter)) {}

  template<typename T, std::predicate<const T&> F>
  void FilteredQueueWriter<T, F>::push(const Target& value) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        std::rethrow_exception(m_exception);
      }
    }
    if(m_filter(value)) {
      m_destination.push(value);
    }
  }

  template<typename T, std::predicate<const T&> F>
  void FilteredQueueWriter<T, F>::push(Target&& value) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        std::rethrow_exception(m_exception);
      }
    }
    if(m_filter(value)) {
      m_destination.push(std::move(value));
    }
  }

  template<typename T, std::predicate<const T&> F>
  void FilteredQueueWriter<T, F>::close(const std::exception_ptr& e) {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_exception) {
        return;
      }
      m_exception = e;
    }
    m_destination.close(e);
  }
}

#endif
