#ifndef BEAM_CONVERTER_QUEUE_WRITER_HPP
#define BEAM_CONVERTER_QUEUE_WRITER_HPP
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /**
   * Used to push converted data onto a QueueWriter.
   * @param <T> The type of data being pushed onto the QueueWriter.
   * @param <C> The type of function performing the conversion.
   */
  template<typename T, typename C>
  class ConverterQueueWriter : public QueueWriter<T> {
    public:
      using Target = typename QueueWriter<T>::Target;

      /**
       * The type of function performing the conversion.
       * @param source The data that was pushed onto the Queue.
       * @return The converted data to push onto the TargetQueue.
       */
      using Converter = C;

      /** The type that the <i>Target</i> is converted to. */
      using Destination = std::invoke_result_t<Converter, const Target&>;

      /**
       * Constructs a ConverterQueueWriter.
       * @param target The target to push the converted data onto.
       * @param converter The function performing the conversion.
       */
      template<typename CF>
      ConverterQueueWriter(ScopedQueueWriter<Destination> target,
        CF&& converter);

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<T>::Break;

    private:
      ScopedQueueWriter<Destination> m_target;
      Converter m_converter;
  };

  /**
   * Builds a ConverterQueueWriter.
   * @param target The target to push the converted data onto.
   * @param converter The function performing the conversion.
   */
  template<typename T, typename C>
  auto MakeConverterQueueWriter(
      ScopedQueueWriter<std::invoke_result_t<C, const T&>> target,
      C&& converter) {
    return std::make_shared<ConverterQueueWriter<T, std::decay_t<C>>>(
      std::move(target), std::forward<C>(converter));
  }

  /**
   * Builds a ConverterQueueWriter used to push tasks.
   * @param target The target to push the converted data onto.
   * @param task The task to perform when a value is pushed onto the
   *        QueueWriter.
   */
  template<typename T, typename C>
  auto MakeTaskConverterQueue(
      ScopedQueueWriter<std::invoke_result_t<C, const T&>> target, C&& task) {
    return MakeConverterWriterQueue(std::move(target),
      [task = std::forward<C>(task)] (auto&& source) {
        return [=, source = std::forward<decltype(source)>(source)] () mutable {
          task(std::forward<decltype(source)>(source));
        };
      });
  }

  template<typename T, typename C>
  template<typename CF>
  ConverterQueueWriter<T, C>::ConverterQueueWriter(
    ScopedQueueWriter<Destination> target, CF&& converter)
    : m_target(std::move(target)),
      m_converter(std::forward<CF>(converter)) {}

  template<typename T, typename C>
  void ConverterQueueWriter<T, C>::Push(const Target& value) {
    m_target.Push(m_converter(value));
  }

  template<typename T, typename C>
  void ConverterQueueWriter<T, C>::Push(Target&& value) {
    m_target.Push(m_converter(std::move(value)));
  }

  template<typename T, typename C>
  void ConverterQueueWriter<T, C>::Break(const std::exception_ptr& e) {
    m_target.Break(e);
  }
}

#endif
