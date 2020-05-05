#ifndef BEAM_FILTERWRITERQUEUE_HPP
#define BEAM_FILTERWRITERQUEUE_HPP
#include <functional>
#include <memory>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /*! \class FilterWriterQueue
      \brief Filters data pushed onto a Queue.
      \tparam SourceType The type of data to filter.
      \tparam DestinationQueueType The type of Queue to push the filtered data
              onto.
   */
  template<typename SourceType,
    typename DestinationQueueType = QueueWriter<SourceType>>
  class FilterWriterQueue : public QueueWriter<SourceType> {
    public:
      using Source = SourceType;

      //! The type of Queue to push the filtered data onto.
      using DestinationQueue = DestinationQueueType;

      //! The function used to perform the filtering.
      /*!
        \param data The data that was pushed onto the Queue.
        \return <code>true</code> iff the data satisfies the filter.
      */
      using FilterFunction = std::function<bool(const Source&)>;

      //! Constructs a FilterQueue.
      /*!
        \param destination The Queue to push the filtered data onto.
        \param filter The function performing the filtering.
      */
      FilterWriterQueue(const std::shared_ptr<DestinationQueue>& destination,
        const FilterFunction& filter);

      virtual ~FilterWriterQueue();

      virtual void Push(const Source& value);

      virtual void Break(const std::exception_ptr& e);
    private:
      std::shared_ptr<DestinationQueue> m_destination;
      FilterFunction m_filter;
  };

  //! Instantiates a FilterWriterQueue.
  /*!
    \param destination The Queue to push the filtered data onto.
    \param filter The function performing the filtering.
  */
  template<typename DestinationQueueType>
  std::shared_ptr<FilterWriterQueue<typename DestinationQueueType::Source,
      DestinationQueueType>> MakeFilterWriterQueue(
      const std::shared_ptr<DestinationQueueType>& destination,
      typename FilterWriterQueue<typename DestinationQueueType::Source,
      DestinationQueueType>::FilterFunction& filter) {
    return std::make_shared<FilterWriterQueue<
      typename DestinationQueueType::Source, DestinationQueueType>>(destination,
      filter);
  }

  template<typename SourceType, typename DestinationQueueType>
  FilterWriterQueue<SourceType, DestinationQueueType>::FilterWriterQueue(
      const std::shared_ptr<DestinationQueue>& destination,
      const FilterFunction& filter)
      : m_destination(destination),
        m_filter(filter) {}

  template<typename SourceType, typename DestinationQueueType>
  FilterWriterQueue<SourceType, DestinationQueueType>::~FilterWriterQueue() {
    Break();
  }

  template<typename SourceType, typename DestinationQueueType>
  void FilterWriterQueue<SourceType, DestinationQueueType>::Push(
      const Source& value) {
    if(!m_filter(value)) {
      return;
    }
    m_destination->Push(value);
  }

  template<typename SourceType, typename DestinationQueueType>
  void FilterWriterQueue<SourceType, DestinationQueueType>::Break(
      const std::exception_ptr& e) {
    m_destination->Break(e);
  }
}

#endif
