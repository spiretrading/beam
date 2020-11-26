#ifndef BEAM_SEQUENCED_VALUE_PUBLISHER_HPP
#define BEAM_SEQUENCED_VALUE_PUBLISHER_HPP
#include <memory>
#include <utility>
#include <vector>
#include <boost/range/iterator_range.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/Queries/QueryInterruptedException.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam::Queries {

  /**
   * Publishes SequencedValues sent via a query.
   * @param <Q> The type of query submitted.
   * @param <V> The type of data received.
   */
  template<typename Q, typename V>
  class SequencedValuePublisher {
    public:

      /** The type of query submitted. */
      using Query = Q;

      /** The type of data received. */
      using Value = SequencedValue<V>;

      /**
       * Constructs a SequencedValuePublisher.
       * @param query The query submitted.
       * @param filter The filter to apply to published values.
       * @param queue The queue to publish the data to.
       */
      SequencedValuePublisher(const Query& query,
        std::unique_ptr<Evaluator> filter, ScopedQueueWriter<Value> queue);

      /** Returns the id of the query. */
      int GetId() const;

      /**
       * Publishes data.
       * @param value The value to publish.
       */
      template<typename T>
      void Push(T&& value);

      /**
       * Publishes data used as part of a query's snapshot.
       * @param begin An iterator to the beginning of the snapshot.
       * @param end An iterator to one past the end of the snapshot.
       */
      template<typename Iterator>
      void PushSnapshot(Iterator begin, Iterator end);

      /** Begins receiving data used to initialize the query. */
      void BeginSnapshot();

      /**
       * Ends the snapshot.
       * @param queryId The query's id.
       */
      void EndSnapshot(int queryId);

      /**
       * Begins recovering data lost from an interrupted query.
       * @return The query to submit to recover the lost data.
       */
      Query BeginRecovery();

      /**
       * Ends the data recovery, resuming normal query publication.
       * @param queryId The resumed query's id.
       */
      void EndRecovery(int queryId);

      /** Breaks the queue receiving the published data. */
      void Break();

      /**
       * Breaks the queue receiving the published data.
       * @param e The exception used to break the queue.
       */
      template<typename E>
      void Break(E&& e);

      /**
       * Breaks the queue receiving the published data.
       * @param e The exception used to break the queue.
       */
      void Break(const std::exception_ptr& e);

    private:
      mutable boost::mutex m_mutex;
      Query m_query;
      std::unique_ptr<Evaluator> m_filter;
      ScopedQueueWriter<Value> m_queue;
      int m_queryId;
      Sequence m_nextSequence;
      std::vector<Value> m_writeLog;

      SequencedValuePublisher(const SequencedValuePublisher&) = delete;
      SequencedValuePublisher& operator =(
        const SequencedValuePublisher&) = delete;
  };

  template<typename Q, typename V>
  SequencedValuePublisher<Q, V>::SequencedValuePublisher(const Query& query,
    std::unique_ptr<Evaluator> filter, ScopedQueueWriter<Value> queue)
    : m_query(query),
      m_filter(std::move(filter)),
      m_queue(std::move(queue)),
      m_queryId(-1),
      m_nextSequence(Sequence::First()) {}

  template<typename Q, typename V>
  int SequencedValuePublisher<Q, V>::GetId() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_queryId;
  }

  template<typename Q, typename V>
  template<typename T>
  void SequencedValuePublisher<Q, V>::Push(T&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_queryId == -1) {
      m_writeLog.emplace_back(std::forward<T>(value));
    } else {
      if(value.GetSequence() >= m_nextSequence &&
          TestFilter(*m_filter, *value)) {
        m_nextSequence = Increment(value.GetSequence());
        m_queue.Push(std::forward<T>(value));
      }
    }
  }

  template<typename Q, typename V>
  template<typename Iterator>
  void SequencedValuePublisher<Q, V>::PushSnapshot(Iterator begin,
      Iterator end) {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& value : boost::iterator_range<Iterator>(begin, end)) {
      if(value.GetSequence() >= m_nextSequence &&
          TestFilter(*m_filter, *value)) {
        m_nextSequence = Increment(value.GetSequence());
        m_queue.Push(std::move(value));
      }
    }
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::BeginSnapshot() {
    auto lock = boost::lock_guard(m_mutex);
    m_queryId = -1;
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::EndSnapshot(int queryId) {
    auto lock = boost::lock_guard(m_mutex);
    m_queryId = queryId;
    auto writeLog = std::vector<Value>();
    writeLog.swap(m_writeLog);
    for(auto& value : writeLog) {
      if(value.GetSequence() >= m_nextSequence &&
          TestFilter(*m_filter, *value)) {
        m_nextSequence = Increment(value.GetSequence());
        m_queue.Push(std::move(value));
      }
    }
  }

  template<typename Q, typename V>
  typename SequencedValuePublisher<Q, V>::Query
      SequencedValuePublisher<Q, V>::BeginRecovery() {
    auto lock = boost::lock_guard(m_mutex);
    m_queryId = -1;
    if(m_query.GetInterruptionPolicy() == InterruptionPolicy::RECOVER_DATA ||
        m_query.GetInterruptionPolicy() ==
        InterruptionPolicy::IGNORE_CONTINUE) {
      auto startPoint = Range::Point();
      if(m_query.GetInterruptionPolicy() ==
          InterruptionPolicy::IGNORE_CONTINUE) {
        startPoint = Sequence::Present();
      } else {
        if(m_nextSequence == Sequence::First()) {
          startPoint = m_query.GetRange().GetStart();
        } else {
          startPoint = m_nextSequence;
        }
      }
      auto recoveryQuery = m_query;
      recoveryQuery.SetRange(startPoint, m_query.GetRange().GetEnd());
      return recoveryQuery;
    } else {
      m_queue.Break(QueryInterruptedException());
      BOOST_THROW_EXCEPTION(QueryInterruptedException());
    }
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::EndRecovery(int queryId) {
    EndSnapshot(queryId);
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::Break() {
    m_queue.Break();
  }

  template<typename Q, typename V>
  template<typename E>
  void SequencedValuePublisher<Q, V>::Break(E&& e) {
    Break(std::make_exception_ptr(std::forward<E>(e)));
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::Break(const std::exception_ptr& e) {
    m_queue.Break(e);
  }
}

#endif
