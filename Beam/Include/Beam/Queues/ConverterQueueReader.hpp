#ifndef BEAM_CONVERTER_QUEUE_READER_HPP
#define BEAM_CONVERTER_QUEUE_READER_HPP
#include <concepts>
#include <type_traits>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Used to convert data popped off a QueueReader.
   * @tparam T The type of data to pop and convert.
   * @tparam C The type of function performing the conversion.
   */
  template<typename T, std::invocable<T&&> C>
  class ConverterQueueReader :
      public QueueReader<std::invoke_result_t<C, T&&>> {
    public:
      using Source =
        typename QueueReader<std::invoke_result_t<C, const T&>>::Source;

      /**
       * The type of function performing the conversion.
       * @param source The value that was popped from the QueueReader.
       * @return The value to pop from this QueueReader.
       */
      using Converter = C;

      /** The type that the <i>Source</i> is converted to. */
      using Target = std::invoke_result_t<Converter, T&&>;

      /**
       * Constructs a ConverterQueueReader.
       * @param source The QueueReader to convert.
       * @param converter Initializes the Converter.
       */
      template<Initializes<C> CF>
      ConverterQueueReader(ScopedQueueReader<T> source, CF&& converter);

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& e) override;
      using QueueReader<std::invoke_result_t<C, T&&>>::close;

    private:
      ScopedQueueReader<T> m_source;
      Converter m_converter;
  };

  template<typename Q, typename C>
  ConverterQueueReader(Q&&, C&&) -> ConverterQueueReader<
    typename dereference_t<Q>::Source, std::remove_cvref_t<C>>;

  /**
   * Returns a ConverterReaderQueue.
   * @param source The QueueReader to convert.
   * @param converter The conversion function to use.
   */
  template<typename Q,
    std::invocable<typename dereference_t<Q>::Source> C>
  auto convert(Q&& source, C&& converter) requires
      IsSubclass<dereference_t<Q>, QueueReader> {
    using Source = typename dereference_t<Q>::Source;
    return std::make_shared<
      ConverterQueueReader<Source, std::remove_cvref_t<C>>>(
        std::forward<Q>(source), std::forward<C>(converter));
  }

  template<typename T, std::invocable<T&&> C>
  template<Initializes<C> CF>
  ConverterQueueReader<T, C>::ConverterQueueReader(
    ScopedQueueReader<T> source, CF&& converter)
    : m_source(std::move(source)),
      m_converter(std::forward<CF>(converter)) {}

  template<typename T, std::invocable<T&&> C>
  typename ConverterQueueReader<T, C>::Source
      ConverterQueueReader<T, C>::pop() {
    return m_converter(m_source.pop());
  }

  template<typename T, std::invocable<T&&> C>
  boost::optional<typename ConverterQueueReader<T, C>::Source>
      ConverterQueueReader<T, C>::try_pop() {
    if(auto v = m_source.try_pop()) {
      return m_converter(std::move(*v));
    }
    return boost::none;
  }

  template<typename T, std::invocable<T&&> C>
  void ConverterQueueReader<T, C>::close(const std::exception_ptr& e) {
    m_source.close(e);
  }
}

#endif
