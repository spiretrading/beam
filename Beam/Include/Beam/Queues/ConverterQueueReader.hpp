#ifndef BEAM_CONVERTER_QUEUE_READER_HPP
#define BEAM_CONVERTER_QUEUE_READER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/compressed_pair.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /**
   * Used to convert data pushed into a Queue.
   * @param <T> The type of data to read from this Queue.
   * @param <S> The Queue to convert.
   * @param <C> The type of function performing the conversion.
   */
  template<typename T, typename S, typename C>
  class ConverterQueueReader : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      /** The Queue to convert. */
      using SourceQueue = S;

      /** The type of function performing the conversion. */
      using Converter = C;

      /**
       * Constructs a ConverterReaderQueue.
       * @param source The Queue to convert.
       * @param converter Initializes the Converter.
       */
      template<typename CF>
      ConverterQueueReader(std::shared_ptr<SourceQueue> source, CF&& converter);

      /**
       * Constructs a ConverterReaderQueue.
       * @param source The Queue to convert.
       */
      ConverterQueueReader(std::shared_ptr<SourceQueue> source);

      bool IsEmpty() const override;

      Target Top() const override;

      void Pop() override;

      void Break(const std::exception_ptr& e) override;

    protected:
      bool IsAvailable() const override;

    private:
      boost::compressed_pair<std::shared_ptr<SourceQueue>, Converter> m_source;
  };

  /**
   * Builds a ConverterReaderQueue.
   * @param source The Queue to convert.
   * @param converter The conversion function to use.
   */
  template<typename T, typename S, typename C>
  auto MakeConverterQueueReader(std::shared_ptr<S> source, C&& converter) {
    return std::make_shared<ConverterQueueReader<T, S, std::decay_t<C>>>(
      std::move(queue), std::forward<C>(converter));
  }

  template<typename T, typename S, typename C>
  template<typename CF>
  ConverterQueueReader<T, S, C>::ConverterQueueReader(
    std::shared_ptr<SourceQueue> source, CF&& converter)
    : m_source(std::move(source), std::forward<CF>(converter)) {}

  template<typename T, typename S, typename C>
  ConverterQueueReader<T, S, C>::ConverterQueueReader(
    std::shared_ptr<SourceQueue> source)
    : ConverterQueueReader(std::move(source), Converter()) {}

  template<typename T, typename S, typename C>
  bool ConverterQueueReader<T, S, C>::IsEmpty() const {
    return m_source.first()->IsEmpty();
  }

  template<typename T, typename S, typename C>
  typename ConverterQueueReader<T, S, C>::Target
      ConverterQueueReader<T, S, C>::Top() const {
    return m_source.second()(m_source.first()->Top());
  }

  template<typename T, typename S, typename C>
  void ConverterQueueReader<T, S, C>::Pop() {
    m_source.first()->Pop();
  }

  template<typename T, typename S, typename C>
  void ConverterQueueReader<T, S, C>::Break(const std::exception_ptr& e) {
    m_source.first()->Break(e);
  }

  template<typename T, typename S, typename C>
  bool ConverterQueueReader<T, S, C>::IsAvailable() const {
    return !m_source.first()->IsEmpty();
  }
}

#endif
