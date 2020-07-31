#ifndef BEAM_FILTER_QUEUE_WRITER_HPP
#define BEAM_FILTER_QUEUE_WRITER_HPP
#include <functional>
#include <memory>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /**
   * Filters data pushed onto a Queue.
   * @param <S> The type of data to filter.
   * @param <D> The type of Queue to push the filtered data onto.
   */
  template<typename S, typename D = QueueWriter<S>>
  class FilterQueueWriter : public QueueWriter<S> {
    public:
      using Source = typename QueueWriter<S>::Source;

      /** The type of Queue to push the filtered data onto. */
      using DestinationQueue = D;

      /**
       * The function used to perform the filtering.
       * @param data The data that was pushed onto the Queue.
       * @return <code>true</code> iff the data satisfies the filter.
       */
      using FilterFunction = std::function<bool(const Source&)>;

      /**
       * Constructs a FilterQueue.
       * @param destination The Queue to push the filtered data onto.
       * @param filter The function performing the filtering.
       */
      FilterQueueWriter(std::shared_ptr<DestinationQueue> destination,
        const FilterFunction& filter);

      ~FilterQueueWriter() override;

      void Push(const Source& value) override;

      void Break(const std::exception_ptr& e) override;

    private:
      std::shared_ptr<DestinationQueue> m_destination;
      FilterFunction m_filter;
  };

  /**
   * Instantiates a FilterWriterQueue.
   * @param destination The Queue to push the filtered data onto.
   * @param filter The function performing the filtering.
   */
  template<typename D>
  auto MakeFilterQueueWriter(std::shared_ptr<D> destination,
      typename FilterQueueWriter<typename D::Source, D>::FilterFunction&
      filter) {
    return std::make_shared<FilterQueueWriter<typename D::Source, D>>(
      std::move(destination), filter);
  }

  template<typename S, typename D>
  FilterQueueWriter<S, D>::FilterQueueWriter(
    std::shared_ptr<DestinationQueue> destination, const FilterFunction& filter)
    : m_destination(std::move(destination)),
      m_filter(filter) {}

  template<typename S, typename D>
  FilterQueueWriter<S, D>::~FilterQueueWriter() {
    Break();
  }

  template<typename S, typename D>
  void FilterQueueWriter<S, D>::Push(const Source& value) {
    if(!m_filter(value)) {
      return;
    }
    m_destination->Push(value);
  }

  template<typename S, typename D>
  void FilterQueueWriter<S, D>::Break(const std::exception_ptr& e) {
    m_destination->Break(e);
  }
}

#endif
