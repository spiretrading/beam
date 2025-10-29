#ifndef BEAM_CONVERTER_QUEUE_WRITER_HPP
#define BEAM_CONVERTER_QUEUE_WRITER_HPP
#include <concepts>
#include <type_traits>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Used to push converted data onto a QueueWriter.
   * @tparam T The type of data being pushed onto the QueueWriter.
   * @tparam C The type of function performing the conversion.
   */
  template<typename T, std::invocable<T&&> C>
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
      using Destination = std::invoke_result_t<Converter, Target&&>;

      /**
       * Constructs a ConverterQueueWriter.
       * @param target The target to push the converted data onto.
       * @param converter The function performing the conversion.
       */
      template<Initializes<C> CF>
      ConverterQueueWriter(
        ScopedQueueWriter<Destination> target, CF&& converter);

      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      using QueueWriter<T>::close;

    private:
      ScopedQueueWriter<Destination> m_target;
      Converter m_converter;
  };

  /**
   * Returns a ConverterQueueWriter.
   * @param target The target to push the converted data onto.
   * @param converter The function performing the conversion.
   */
  template<typename T, std::invocable<T&&> C>
  auto convert(ScopedQueueWriter<std::invoke_result_t<C, T&&>> target,
      C&& converter) {
    return std::make_shared<ConverterQueueWriter<T, std::remove_cvref_t<C>>>(
      std::move(target), std::forward<C>(converter));
  }

  template<typename T, std::invocable<T&&> C>
  template<Initializes<C> CF>
  ConverterQueueWriter<T, C>::ConverterQueueWriter(
    ScopedQueueWriter<Destination> target, CF&& converter)
    : m_target(std::move(target)),
      m_converter(std::forward<CF>(converter)) {}

  template<typename T, std::invocable<T&&> C>
  void ConverterQueueWriter<T, C>::push(const Target& value) {
    m_target.push(m_converter(value));
  }

  template<typename T, std::invocable<T&&> C>
  void ConverterQueueWriter<T, C>::push(Target&& value) {
    m_target.push(m_converter(std::move(value)));
  }

  template<typename T, std::invocable<T&&> C>
  void ConverterQueueWriter<T, C>::close(const std::exception_ptr& e) {
    m_target.close(e);
  }
}

#endif
