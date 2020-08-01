#ifndef BEAM_CONVERTER_QUEUE_WRITER_HPP
#define BEAM_CONVERTER_QUEUE_WRITER_HPP
#include <functional>
#include <memory>
#include <type_traits>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /**
   * Used to convert data pushed from a source into another type.
   * @param <S> The type of data being pushed onto the Queue.
   * @param <T> The Queue to push the converted data to.
   * @param <C> The type of function performing the conversion.
   */
  template<typename S, typename T, typename C>
  class ConverterQueueWriter : public QueueWriter<S> {
    public:
      using Source = typename QueueWriter<S>::Source;

      /** The Queue to push the converted data to. */
      using TargetQueue = T;

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
      ConverterQueueWriter(std::shared_ptr<TargetQueue> target, CF&& converter);

      ~ConverterQueueWriter() override;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

      using QueueWriter<Source>::Break;

    private:
      std::shared_ptr<TargetQueue> m_target;
      Converter m_converter;
  };

  /**
   * Builds a ConverterQueueWriter.
   * @param target The target to push the converted data onto.
   * @param converter The function performing the conversion.
   */
  template<typename S, typename T, typename C>
  auto MakeConverterQueueWriter(std::shared_ptr<T> target, C&& converter) {
    return std::make_shared<ConverterQueueWriter<S, T, std::decay_t<C>>>(
      std::move(target), std::forward<C>(converter));
  }

  /**
   * Builds a ConverterWriterQueue used to push tasks.
   * @param target The target to push the converted data onto.
   * @param task The task to perform when a value is pushed onto the Queue.
   */
  template<typename S, typename T>
  auto MakeTaskConverterQueue(std::shared_ptr<T> target,
      const std::function<void (const S&)>& task) {
    return MakeConverterWriterQueue(std::move(target),
      [=] (const S& source) -> std::function<void ()> {
        return [=] {
          task(source);
        };
      });
  }

  template<typename S, typename T, typename C>
  template<typename CF>
  ConverterQueueWriter<S, T, C>::ConverterQueueWriter(
    std::shared_ptr<TargetQueue> target, CF&& converter)
    : m_target(std::move(target)),
      m_converter(std::forward<CF>(converter)) {}

  template<typename S, typename T, typename C>
  ConverterQueueWriter<S, T, C>::~ConverterQueueWriter() {
    Break();
  }

  template<typename S, typename T, typename C>
  void ConverterQueueWriter<S, T, C>::Push(const Source& value) {
    m_target->Push(m_converter(value));
  }

  template<typename S, typename T, typename C>
  void ConverterQueueWriter<S, T, C>::Push(Source&& value) {
    m_target->Push(m_converter(std::move(value)));
  }

  template<typename S, typename T, typename C>
  void ConverterQueueWriter<S, T, C>::Break(const std::exception_ptr& e) {
    m_target->Break(e);
  }
}

#endif
