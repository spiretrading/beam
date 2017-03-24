#ifndef BEAM_CONVERTERWRITERQUEUE_HPP
#define BEAM_CONVERTERWRITERQUEUE_HPP
#include <memory>
#include <type_traits>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /*! \class ConverterWriterQueue
      \brief Used to convert data pushed from a source into another type.
      \tparam SourceType The type of data being pushed onto the Queue.
      \tparam TargetQueueType The Queue to push the converted data to.
      \tparam ConverterType The type of function performing the conversion.
   */
  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  class ConverterWriterQueue : public QueueWriter<SourceType> {
    public:

      //! The type of data being pushed onto the Queue.
      using Source = SourceType;

      //! The Queue to push the converted data to.
      using TargetQueue = TargetQueueType;

      //! The type of function performing the conversion.
      /*!
        \param source The data that was pushed onto the Queue.
        \return The converted data to push onto the TargetQueue.
      */
      using Converter = ConverterType;

      //! Constructs a ConverterQueue.
      /*!
        \param target The target to push the converted data onto.
        \param converter The function performing the conversion.
      */
      template<typename ConverterForward>
      ConverterWriterQueue(const std::shared_ptr<TargetQueue>& target,
        ConverterForward&& converter);

      virtual ~ConverterWriterQueue() override;

      virtual void Push(const Source& value) override;

      virtual void Push(Source&& value) override;

      virtual void Break(const std::exception_ptr& e) override;

      using QueueWriter<Source>::Break;
    private:
      std::shared_ptr<TargetQueue> m_target;
      Converter m_converter;
  };

  //! Builds a ConverterWriterQueue.
  /*!
    \param target The target to push the converted data onto.
    \param converter The function performing the conversion.
  */
  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  auto MakeConverterWriterQueue(const std::shared_ptr<TargetQueueType>& target,
      ConverterType&& converter) {
    return std::make_shared<ConverterWriterQueue<SourceType, TargetQueueType,
      std::decay_t<ConverterType>>>(target,
      std::forward<ConverterType>(converter));
  }

  //! Builds a ConverterWriterQueue used to push tasks.
  /*!
    \param target The target to push the converted data onto.
    \param task The task to perform when a value is pushed onto the Queue.
  */
  template<typename SourceType, typename TargetQueueType>
  auto MakeTaskConverterQueue(const std::shared_ptr<TargetQueueType>& target,
      const std::function<void (const SourceType&)>& task) {
    auto converter =
      [=] (const SourceType& source) -> std::function<void ()> {
        return [=] {
          task(source);
        };
      };
    return MakeConverterWriterQueue(target, std::move(converter));
  }

  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  template<typename ConverterForward>
  ConverterWriterQueue<SourceType, TargetQueueType, ConverterType>::
      ConverterWriterQueue(const std::shared_ptr<TargetQueue>& target,
      ConverterForward&& converter)
      : m_target(target),
        m_converter(std::forward<ConverterForward>(converter)) {}

  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  ConverterWriterQueue<SourceType, TargetQueueType, ConverterType>::
      ~ConverterWriterQueue() {
    Break();
  }

  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  void ConverterWriterQueue<SourceType, TargetQueueType, ConverterType>::Push(
      const Source& value) {
    m_target->Push(m_converter(value));
  }

  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  void ConverterWriterQueue<SourceType, TargetQueueType, ConverterType>::Push(
      Source&& value) {
    m_target->Push(m_converter(std::move(value)));
  }

  template<typename SourceType, typename TargetQueueType,
    typename ConverterType>
  void ConverterWriterQueue<SourceType, TargetQueueType, ConverterType>::Break(
      const std::exception_ptr& e) {
    m_target->Break(e);
  }
}

#endif
