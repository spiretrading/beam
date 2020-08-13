#ifndef BEAM_QUEUES_HPP
#define BEAM_QUEUES_HPP

namespace Beam {
  template<typename T> class AbstractQueue;
  template<typename T> class AggregateQueueReader;
  class BasePublisher;
  class BaseQueue;
  class CallbackQueue;
  template<typename T, typename C, typename B> class CallbackQueueWriter;
  template<typename T, typename C> class ConverterQueueReader;
  template<typename T, typename C> class ConverterQueueWriter;
  template<typename T, typename F> class FilteredQueueReader;
  template<typename T, typename F> class FilteredQueueWriter;
  template<typename T> class MultiQueueWriter;
  class PipeBrokenException;
  template<typename T> class Publisher;
  template<typename T> class Queue;
  template<typename T, typename U> class QueuePipe;
  template<typename T> class QueueReader;
  template<typename T, typename Q> class QueueReaderPublisher;
  template<typename T> class QueueWriter;
  template<typename T> class QueueWriterPublisher;
  template<typename Q> class ScopedBaseQueue;
  template<typename T, typename Q> class ScopedQueueReader;
  template<typename T, typename Q> class ScopedQueueWriter;
  template<typename T, typename S> class SequencePublisher;
  template<typename T, typename S> class SnapshotPublisher;
  template<typename T> class StatePublisher;
  template<typename T> class StateQueue;
  template<typename K, typename V> class TablePublisher;
  template<typename K, typename V> class TaggedQueueReader;
  class TaskQueue;
  template<typename V, typename S> class ValueSnapshotPublisher;
  template<typename T> class WeakQueueWriter;
}

#endif
