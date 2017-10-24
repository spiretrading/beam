#ifndef BEAM_QUEUES_HPP
#define BEAM_QUEUES_HPP

namespace Beam {
  template<typename T> class AbstractQueue;
  template<typename PublisherType> class AggregatePublisher;
  template<typename T> class AggregateQueue;
  template<typename T> class AliasQueue;
  class BaseCallbackWriterQueue;
  class BasePublisher;
  class BaseQueue;
  class CallbackQueue;
  template<typename T> class CallbackWriterQueue;
  template<typename TargetType, typename SourceQueueType,
    typename ConverterType> class ConverterReaderQueue;
  template<typename SourceType, typename TargetQueueType,
    typename ConverterType> class ConverterWriterQueue;
  template<typename PublisherType> class FilteredPublisher;
  template<typename SourceType, typename DestinationQueueType>
    class FilterWriterQueue;
  template<typename T> class MultiQueueReader;
  template<typename T> class MultiQueueWriter;
  class PipeBrokenException;
  template<typename T> class Publisher;
  template<typename T> class Queue;
  template<typename PublisherType> class QueuePublisher;
  template<typename T> class QueueReader;
  template<typename T> class QueueWriter;
  template<typename T, typename SequenceType> class SequencePublisher;
  template<typename T, typename SnapshotType> class SnapshotPublisher;
  template<typename T> class StatePublisher;
  template<typename T> class StateQueue;
  template<typename KeyType, typename ValueType> struct TableEntry;
  template<typename KeyType, typename ValueType> class TablePublisher;
  template<typename KeyType, typename ValueType> class TaggedQueue;
  class TaskQueue;
  template<typename ValueType, typename SnapshotType>
    class ValueSnapshotPublisher;
  template<typename T> class WeakQueue;
}

#endif
