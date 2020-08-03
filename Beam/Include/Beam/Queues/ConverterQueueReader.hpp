#ifndef BEAM_CONVERTER_QUEUE_READER_HPP
#define BEAM_CONVERTER_QUEUE_READER_HPP
#include <type_traits>
#include <utility>
#include <boost/compressed_pair.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"

namespace Beam {

  /**
   * Used to convert data pushed into a QueueReader.
   * @param <T> The type of data to pop and convert.
   * @param <C> The type of function performing the conversion.
   */
  template<typename T, typename C>
  class ConverterQueueReader :
      public QueueReader<std::invoke_result_t<C, const T&>> {
    public:
      using Target = typename QueueReader<
        std::invoke_result_t<C, const T&>>::Target;

      /**
       * The type of function performing the conversion.
       * @param target The value that was popped from the QueueReader.
       * @return The value to pop from this QueueReader.
       */
      using Converter = C;

      /** The type that the <i>Target</i> is converted to. */
      using Source = std::invoke_result_t<Converter, const T&>;

      /**
       * Constructs a ConverterReaderQueue.
       * @param source The QueueReader to convert.
       * @param converter Initializes the Converter.
       */
      template<typename CF>
      ConverterQueueReader(ScopedQueueReader<T> source, CF&& converter);

      bool IsEmpty() const override;

      Target Pop() override;

      void Break(const std::exception_ptr& e) override;

    private:
      boost::compressed_pair<ScopedQueueReader<T>, Converter> m_source;

      ConverterQueueReader(const ConverterQueueReader&) = delete;
      ConverterQueueReader& operator =(const ConverterQueueReader&) = delete;
  };

  template<typename QueueReader, typename C>
  ConverterQueueReader(QueueReader&&, C&&) -> ConverterQueueReader<
    typename GetTryDereferenceType<QueueReader>::Target, std::decay_t<C>>;

  /**
   * Builds a ConverterReaderQueue.
   * @param source The QueueReader to convert.
   * @param converter The conversion function to use.
   */
  template<typename T, typename C>
  auto MakeConverterQueueReader(ScopedQueueReader<T> source, C&& converter) {
    return std::make_shared<ConverterQueueReader<T, std::decay_t<C>>>(
      std::move(queue), std::forward<C>(converter));
  }

  template<typename T, typename C>
  template<typename CF>
  ConverterQueueReader<T, C>::ConverterQueueReader(ScopedQueueReader<T> source,
    CF&& converter)
    : m_source(std::move(source), std::forward<CF>(converter)) {}

  template<typename T, typename C>
  bool ConverterQueueReader<T, C>::IsEmpty() const {
    return m_source.first().IsEmpty();
  }

  template<typename T, typename C>
  typename ConverterQueueReader<T, C>::Target
      ConverterQueueReader<T, C>::Pop() {
    return m_source.second()(m_source.first().Pop());
  }

  template<typename T, typename C>
  void ConverterQueueReader<T, C>::Break(const std::exception_ptr& e) {
    m_source.first().Break(e);
  }
}

#endif
