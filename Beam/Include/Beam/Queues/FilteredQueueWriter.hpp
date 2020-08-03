#ifndef BEAM_FILTERED_QUEUE_WRITER_HPP
#define BEAM_FILTERED_QUEUE_WRITER_HPP
#include <memory>
#include <type_traits>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /**
   * Filters data pushed onto a Queue.
   * @param <T> The type of data to filter.
   * @param <F> The type used to filter the data.
   */
  template<typename T, typename F>
  class FilteredQueueWriter : public QueueWriter<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /** The type used to filter the data. */
      using Filter = F;

      /**
       * Constructs a FilteredQueueWriter.
       * @param destination The QueueWriter to push the filtered data onto.
       * @param filter The function performing the filtering.
       */
      template<typename FF>
      FilteredQueueWriter(ScopedQueueWriter<Source> destination, FF&& filter);

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      ScopedQueueWriter<Source> m_destination;
      Filter m_filter;
  };

  template<typename Source, typename F>
  FilteredQueueWriter(ScopedQueueWriter<Source>, F&&) ->
    FilteredQueueWriter<Source, std::decay_t<F>>;

  /**
   * Makes a FilteredWriterQueue.
   * @param destination The QueueWriter to push the filtered data onto.
   * @param filter The function performing the filtering.
   */
  template<typename Source, typename F>
  auto MakeFilteredQueueWriter(ScopedQueueWriter<Source> destination,
      F&& filter) {
    return std::make_shared<FilteredQueueWriter<Source, std::decay_t<F>>>(
      std::move(destination), std::forward<F>(filter));
  }

  template<typename T, typename F>
  template<typename FF>
  FilteredQueueWriter<T, F>::FilteredQueueWriter(
    ScopedQueueWriter<Source> destination, FF&& filter)
    : m_destination(std::move(destination)),
      m_filter(std::forward<FF>(filter)) {}

  template<typename T, typename F>
  void FilteredQueueWriter<T, F>::Push(const Source& value) {
    if(!m_filter(value)) {
      return;
    }
    m_destination.Push(value);
  }

  template<typename T, typename F>
  void FilteredQueueWriter<T, F>::Push(Source&& value) {
    if(!m_filter(value)) {
      return;
    }
    m_destination.Push(std::move(value));
  }

  template<typename T, typename F>
  void FilteredQueueWriter<T, F>::Break(const std::exception_ptr& e) {
    m_destination.Break(e);
  }
}

#endif
