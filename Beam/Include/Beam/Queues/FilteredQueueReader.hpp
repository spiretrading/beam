#ifndef BEAM_FILTERED_QUEUE_READER_HPP
#define BEAM_FILTERED_QUEUE_READER_HPP
#include <concepts>
#include <type_traits>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Filters data popped off a QueueReader.
   * @tparam T The type of data to filter.
   * @tparam F The predicate used to filter the data.
   */
  template<typename T, std::predicate<const T&> F>
  class FilteredQueueReader : public QueueReader<T> {
    public:
      using Source = typename QueueReader<T>::Source;

      /** The predicate used to filter the data. */
      using Filter = F;

      /**
       * Constructs a FilteredQueueReader.
       * @param source The QueueReader to filter.
       * @param filter The function used to filter values.
       */
      template<Initializes<F> FF>
      FilteredQueueReader(ScopedQueueReader<T> source, FF&& filter);

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& e) override;
      using QueueReader<T>::close;

    private:
      ScopedQueueReader<Source> m_source;
      Filter m_filter;
  };

  template<typename Q, typename F>
  FilteredQueueReader(Q&&, F&&) -> FilteredQueueReader<
    typename dereference_t<Q>::Source, std::remove_cvref_t<F>>;

  /**
   * Returns a FilteredReaderQueue.
   * @param source The QueueReader to convert.
   * @param filter The function used to filter values.
   */
  template<
    typename Q, std::predicate<const typename dereference_t<Q>::Source&> F>
  auto filter(Q&& source, F&& filter) {
    using Source = typename dereference_t<Q>::Source;
    return std::make_shared<
      FilteredQueueReader<Source, std::remove_cvref_t<F>>>(
        std::forward<Q>(source), std::forward<F>(filter));
  }

  template<typename T, std::predicate<const T&> F>
  template<Initializes<F> FF>
  FilteredQueueReader<T, F>::FilteredQueueReader(
    ScopedQueueReader<T> source, FF&& filter)
    : m_source(std::move(source)),
      m_filter(std::forward<FF>(filter)) {}

  template<typename T, std::predicate<const T&> F>
  typename FilteredQueueReader<T, F>::Source FilteredQueueReader<T, F>::pop() {
    while(true) {
      auto value = m_source.pop();
      if(m_filter(value)) {
        return value;
      }
    }
  }

  template<typename T, std::predicate<const T&> F>
  boost::optional<typename FilteredQueueReader<T, F>::Source>
      FilteredQueueReader<T, F>::try_pop() {
    while(true) {
      if(auto value = m_source.try_pop()) {
        if(m_filter(*value)) {
          return value;
        }
      } else {
        return boost::none;
      }
    }
  }

  template<typename T, std::predicate<const T&> F>
  void FilteredQueueReader<T, F>::close(const std::exception_ptr& e) {
    m_source.close(e);
  }
}

#endif
