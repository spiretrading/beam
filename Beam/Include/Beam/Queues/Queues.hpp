#ifndef BEAM_QUEUES_HPP
#define BEAM_QUEUES_HPP

namespace Beam {
  template<typename T> class AbstractQueue;
  template<typename T> class AggregateQueueReader;
  template<typename T> class AliasQueueWriter;
  class BasePublisher;
  class BaseQueue;
  class CallbackQueue;
  template<typename T, typename C, typename B> class CallbackQueueWriter;
  template<typename T, typename C> class ConverterQueueReader;
  template<typename T, typename C> class ConverterQueueWriter;
  template<typename S, typename D> class FilterQueueWriter;
  template<typename T> class MultiQueueReader;
  template<typename T> class MultiQueueWriter;
  class PipeBrokenException;
  template<typename T> class Publisher;
  template<typename T> class Queue;
  template<typename T> class QueueReader;
  template<typename T> class QueueWriter;
  class ScopedBaseQueue;
  template<typename T> class ScopedQueueReader;
  template<typename T> class ScopedQueueWriter;
  template<typename T, typename S> class SequencePublisher;
  template<typename T, typename S> class SnapshotPublisher;
  template<typename T> class StatePublisher;
  template<typename T> class StateQueue;
  template<typename K, typename V> class TablePublisher;
  template<typename K, typename V> class TaggedQueueReader;
  class TaskQueue;
  template<typename V, typename S> class ValueSnapshotPublisher;
  template<typename T> class WeakQueue;
}

#endif
