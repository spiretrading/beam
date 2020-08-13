#ifndef BEAM_CONVERTER_QUEUE_READER_HPP
#define BEAM_CONVERTER_QUEUE_READER_HPP
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"

namespace Beam {

  /**
   * Used to convert data popped off a QueueReader.
   * @param <T> The type of data to pop and convert.
   * @param <C> The type of function performing the conversion.
   */
  template<typename T, typename C>
  class ConverterQueueReader :
      public QueueReader<std::invoke_result_t<C, const T&>> {
    public:
      using Source = typename QueueReader<
        std::invoke_result_t<C, const T&>>::Source;

      /**
       * The type of function performing the conversion.
       * @param source The value that was popped from the QueueReader.
       * @return The value to pop from this QueueReader.
       */
      using Converter = C;

      /** The type that the <i>Source</i> is converted to. */
      using Target = std::invoke_result_t<Converter, const T&>;

      /**
       * Constructs a ConverterQueueReader.
       * @param source The QueueReader to convert.
       * @param converter Initializes the Converter.
       */
      template<typename CF>
      ConverterQueueReader(ScopedQueueReader<T> source, CF&& converter);

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Break(const std::exception_ptr& e) override;

      using QueueReader<std::invoke_result_t<C, const T&>>::Break;

    private:
      ScopedQueueReader<T> m_source;
      Converter m_converter;
  };

  template<typename QueueReader, typename C>
  ConverterQueueReader(QueueReader&&, C&&) -> ConverterQueueReader<
    typename GetTryDereferenceType<QueueReader>::Source, std::decay_t<C>>;

  /**
   * Builds a ConverterReaderQueue.
   * @param source The QueueReader to convert.
   * @param converter The conversion function to use.
   */
  template<typename QueueReader, typename C>
  auto MakeConverterQueueReader(QueueReader&& source, C&& converter) {
    using Source = typename GetTryDereferenceType<QueueReader>::Source;
    return std::make_shared<ConverterQueueReader<Source, std::decay_t<C>>>(
      std::forward<QueueReader>(source), std::forward<C>(converter));
  }

  template<typename T, typename C>
  template<typename CF>
  ConverterQueueReader<T, C>::ConverterQueueReader(ScopedQueueReader<T> source,
    CF&& converter)
    : m_source(std::move(source)),
      m_converter(std::forward<CF>(converter)) {}

  template<typename T, typename C>
  typename ConverterQueueReader<T, C>::Source
      ConverterQueueReader<T, C>::Pop() {
    return m_converter(m_source.Pop());
  }

  template<typename T, typename C>
  boost::optional<typename ConverterQueueReader<T, C>::Source>
      ConverterQueueReader<T, C>::TryPop() {
    auto v = m_source.TryPop();
    if(v) {
      return m_converter(std::move(*v));
    }
    return boost::none;
  }

  template<typename T, typename C>
  void ConverterQueueReader<T, C>::Break(const std::exception_ptr& e) {
    m_source.Break(e);
  }
}

#endif
