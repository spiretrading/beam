#ifndef BEAM_CONVERTER_QUEUE_WRITER_HPP
#define BEAM_CONVERTER_QUEUE_WRITER_HPP
#include <type_traits>
#include <utility>
#include <boost/compressed_pair.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /**
   * Used to convert data pushed from a source into another type.
   * @param <T> The type of data being pushed onto the QueueWriter.
   * @param <C> The type of function performing the conversion.
   */
  template<typename T, typename C>
  class ConverterQueueWriter : public QueueWriter<T> {
    public:
      using Source = typename QueueWriter<T>::Source;

      /**
       * The type of function performing the conversion.
       * @param source The data that was pushed onto the Queue.
       * @return The converted data to push onto the TargetQueue.
       */
      using Converter = C;

      /**
       * Constructs a ConverterQueue.
       * @param target The target to push the converted data onto.
       * @param converter The function performing the conversion.
       */
      template<typename CF>
      ConverterQueueWriter(ScopedQueueWriter<Source> target, CF&& converter);

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<Source>::Break;

    private:
      ScopedQueueWriter<Source> m_target;
      Converter m_converter;
  };

  /**
   * Builds a ConverterQueueWriter.
   * @param target The target to push the converted data onto.
   * @param converter The function performing the conversion.
   */
  template<typename T, typename C>
  auto MakeConverterQueueWriter(ScopedQueueWriter<T> target, C&& converter) {
    return std::make_shared<ConverterQueueWriter<T, std::decay_t<C>>>(
      std::move(target), std::forward<C>(converter));
  }

  /**
   * Builds a ConverterWriterQueue used to push tasks.
   * @param target The target to push the converted data onto.
   * @param task The task to perform when a value is pushed onto the
   *        QueueWriter.
   */
  template<typename T, typename F>
  auto MakeTaskConverterQueue(ScopedQueueWriter<T> target, F&& task) {
    return MakeConverterWriterQueue(std::move(target),
      [task = std::forward<F>(task)] (const T& source) {
        return [=] {
          task(source);
        };
      });
  }

  template<typename T, typename C>
  template<typename CF>
  ConverterQueueWriter<T, C>::ConverterQueueWriter(
    ScopedQueueWriter<Source> target, CF&& converter)
    : m_target(std::move(target)),
      m_converter(std::forward<CF>(converter)) {}

  template<typename T, typename C>
  void ConverterQueueWriter<T, C>::Push(const Source& value) {
    m_target.Push(m_converter(value));
  }

  template<typename T, typename C>
  void ConverterQueueWriter<T, C>::Push(Source&& value) {
    m_target.Push(m_converter(std::move(value)));
  }

  template<typename T, typename C>
  void ConverterQueueWriter<T, C>::Break(const std::exception_ptr& e) {
    m_target.Break(e);
  }
}

#endif
