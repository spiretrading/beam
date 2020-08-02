#ifndef BEAM_CONVERTER_QUEUE_READER_HPP
#define BEAM_CONVERTER_QUEUE_READER_HPP
#include <type_traits>
#include <utility>
#include <boost/compressed_pair.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"

namespace Beam {

  /**
   * Used to convert data pushed into a QueueReader.
   * @param <T> The type of data to read from this QueueReader.
   * @param <C> The type of function performing the conversion.
   */
  template<typename T, typename C>
  class ConverterQueueReader : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      /** The type of function performing the conversion. */
      using Converter = C;

      /**
       * Constructs a ConverterReaderQueue.
       * @param source The QueueReader to convert.
       * @param converter Initializes the Converter.
       */
      template<typename CF>
      ConverterQueueReader(ScopedQueueReader<Target> source, CF&& converter);

      bool IsEmpty() const override;

      Target Pop() override;

      void Break(const std::exception_ptr& e) override;

    private:
      boost::compressed_pair<ScopedQueueReader<Target>, Converter> m_source;
  };

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
  ConverterQueueReader<T, C>::ConverterQueueReader(
    ScopedQueueReader<Target> source, CF&& converter)
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
