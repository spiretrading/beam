#ifndef BEAM_CONVERTERREADERQUEUE_HPP
#define BEAM_CONVERTERREADERQUEUE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/compressed_pair.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /*! \class ConverterReaderQueue
      \brief Used to convert data pushed into a Queue.
      \tparam TargetType The type of data to read from this Queue.
      \tparam SourceQueueType The Queue to convert.
      \tparam ConverterType The type of function performing the conversion.
   */
  template<typename TargetType, typename SourceQueueType,
      typename ConverterType>
  class ConverterReaderQueue : public QueueReader<TargetType> {
    public:

      //! The type of data to read from this Queue.
      using Target = TargetType;

      //! The Queue to convert.
      using SourceQueue = SourceQueueType;

      //! The type of function performing the conversion.
      using Converter = ConverterType;

      //! Constructs a ConverterReaderQueue.
      /*!
        \param source The Queue to convert.
        \param converter Initializes the Converter.
      */
      template<typename ConverterForward>
      ConverterReaderQueue(std::shared_ptr<SourceQueue> source,
        ConverterForward&& converter);

      //! Constructs a ConverterReaderQueue.
      /*!
        \param source The Queue to convert.
      */
      ConverterReaderQueue(std::shared_ptr<SourceQueue> source);

      virtual bool IsEmpty() const;

      virtual Target Top() const;

      virtual void Pop();

      virtual void Break(const std::exception_ptr& e);

    protected:
      virtual bool IsAvailable() const;

    private:
      boost::compressed_pair<std::shared_ptr<SourceQueue>, Converter> m_source;
  };

  //! Builds a ConverterReaderQueue.
  /*!
    \param queue The Queue to convert.
    \param converter The conversion function to use.
  */
  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  std::shared_ptr<ConverterReaderQueue<TargetType, SourceQueueType,
      ConverterType>> MakeConverterReaderQueue(
      const std::shared_ptr<SourceQueueType>& queue,
      ConverterType&& converter) {
    return std::make_shared<ConverterReaderQueue<TargetType, SourceQueueType,
      typename std::decay<ConverterType>::type>>(queue,
      std::forward<ConverterType>(converter));
  }

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  template<typename ConverterForward>
  ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::
      ConverterReaderQueue(std::shared_ptr<SourceQueue> source,
      ConverterForward&& converter)
      : m_source(std::move(source),
          std::forward<ConverterForward>(converter)) {}

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::
      ConverterReaderQueue(std::shared_ptr<SourceQueue> source)
      : ConverterReaderQueue{std::move(source), Converter{}} {}

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  bool ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::
      IsEmpty() const {
    return m_source.first()->IsEmpty();
  }

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  typename ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::
      Target ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::
      Top() const {
    return m_source.second()(m_source.first()->Top());
  }

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  void ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::Pop() {
    m_source.first()->Pop();
  }

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  void ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::Break(
      const std::exception_ptr& e) {
    m_source.first()->Break(e);
  }

  template<typename TargetType, typename SourceQueueType,
    typename ConverterType>
  bool ConverterReaderQueue<TargetType, SourceQueueType, ConverterType>::
      IsAvailable() const {
    return !m_source.first()->IsEmpty();
  }
}

#endif
