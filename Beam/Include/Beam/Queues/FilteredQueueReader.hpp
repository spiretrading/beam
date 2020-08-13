#ifndef BEAM_FILTERED_QUEUE_READER_HPP
#define BEAM_FILTERED_QUEUE_READER_HPP
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"

namespace Beam {

  /**
   * Filters data popped off a QueueReader.
   * @param <T> The type of data to filter.
   * @param <F> The type used to filter the data.
   */
  template<typename T, typename F>
  class FilteredQueueReader : public QueueReader<T> {
    public:
      using Source = typename QueueReader<T>::Source;

      /**
       * The type used to filter the data.
       * @param value The value to test.
       * @return <code>false</code> iff the <i>value</i> should be filtered out.
       */
      using Filter = F;

      /**
       * Constructs a FilteredQueueReader.
       * @param source The QueueReader to filter.
       * @param filter The function used to filter values.
       */
      template<typename FF>
      FilteredQueueReader(ScopedQueueReader<T> source, FF&& filter);

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Break(const std::exception_ptr& e) override;

      using QueueReader<T>::Break;

    private:
      ScopedQueueReader<Source> m_source;
      Filter m_filter;
  };

  template<typename QueueReader, typename F>
  FilteredQueueReader(QueueReader&&, F&&) -> FilteredQueueReader<
    typename GetTryDereferenceType<QueueReader>::Source, std::decay_t<F>>;

  /**
   * Builds a FilteredReaderQueue.
   * @param source The QueueReader to convert.
   * @param filter The function used to filter values.
   */
  template<typename QueueReader, typename F>
  auto MakeFilteredQueueReader(QueueReader&& source, F&& filter) {
    using Source = typename GetTryDereferenceType<QueueReader>::Source;
    return std::make_shared<FilteredQueueReader<Source, std::decay_t<F>>>(
      std::forward<QueueReader>(source), std::forward<F>(filter));
  }

  template<typename T, typename F>
  template<typename FF>
  FilteredQueueReader<T, F>::FilteredQueueReader(ScopedQueueReader<T> source,
    FF&& filter)
    : m_source(std::move(source)),
      m_filter(std::forward<FF>(filter)) {}

  template<typename T, typename F>
  typename FilteredQueueReader<T, F>::Source FilteredQueueReader<T, F>::Pop() {
    while(true) {
      auto value = m_source.Pop();
      if(m_filter(value)) {
        return value;
      }
    }
  }

  template<typename T, typename F>
  boost::optional<typename FilteredQueueReader<T, F>::Source>
      FilteredQueueReader<T, F>::TryPop() {
    while(true) {
      auto value = m_source.TryPop();
      if(!value) {
        return boost::none;
      }
      if(m_filter(*value)) {
        return value;
      }
    }
  }

  template<typename T, typename F>
  void FilteredQueueReader<T, F>::Break(const std::exception_ptr& e) {
    m_source.Break(e);
  }
}

#endif
