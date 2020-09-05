#ifndef BEAM_SEQUENCEDVALUEPUBLISHER_HPP
#define BEAM_SEQUENCEDVALUEPUBLISHER_HPP
#include <memory>
#include <utility>
#include <vector>
#include <boost/noncopyable.hpp>
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

namespace Beam {
namespace Queries {

  /*! \class SequencedValuePublisher
      \brief Publishes SequencedValues sent via a query.
      \tparam QueryType The type of query submitted.
      \tparam ValueType The type of data received.
   */
  template<typename QueryType, typename ValueType>
  class SequencedValuePublisher : private boost::noncopyable {
    public:

      //! The type of query submitted.
      using Query = QueryType;

      //! The type of data received.
      using Value = SequencedValue<ValueType>;

      //! Constructs a SequencedValuePublisher.
      /*!
        \param query The query submitted.
        \param filter The filter to apply to published values.
        \param queue The queue to publish the data to.
      */
      SequencedValuePublisher(const Query& query,
        std::unique_ptr<Evaluator> filter, ScopedQueueWriter<Value> queue);

      //! Returns the id of the query.
      int GetId() const;

      //! Publishes data.
      /*!
        \param value The value to publish.
      */
      template<typename ValueForward>
      void Push(ValueForward&& value);

      //! Publishes data used as part of a query's snapshot.
      /*!
        \param begin An iterator to the beginning of the snapshot.
        \param end An iterator to one past the end of the snapshot.
      */
      template<typename ForwardIterator>
      void PushSnapshot(ForwardIterator begin, ForwardIterator end);

      //! Breaks the queue receiving the published data.
      void Break();

      //! Begins receiving data used to initialize the query.
      void BeginSnapshot();

      //! Ends the snapshot.
      /*!
        \param queryId The query's id.
      */
      void EndSnapshot(int queryId);

      //! Begins recovering data lost from an interrupted query.
      /*!
        \return The query to submit to recover the lost data.
      */
      Query BeginRecovery();

      //! Ends the data recovery, resuming normal query publication.
      /*!
        \param queryId The resumed query's id.
      */
      void EndRecovery(int queryId);

    private:
      mutable boost::mutex m_mutex;
      Query m_query;
      std::unique_ptr<Evaluator> m_filter;
      ScopedQueueWriter<Value> m_queue;
      int m_queryId;
      Sequence m_nextSequence;
      std::vector<Value> m_writeLog;
  };

  template<typename QueryType, typename ValueType>
  SequencedValuePublisher<QueryType, ValueType>::SequencedValuePublisher(
      const Query& query, std::unique_ptr<Evaluator> filter,
      ScopedQueueWriter<Value> queue)
      : m_query(query),
        m_filter(std::move(filter)),
        m_queue(std::move(queue)),
        m_queryId(-1),
        m_nextSequence(Sequence::First()) {}

  template<typename QueryType, typename ValueType>
  int SequencedValuePublisher<QueryType, ValueType>::GetId() const {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    auto queryId = m_queryId;
    return queryId;
  }

  template<typename QueryType, typename ValueType>
  template<typename ValueForward>
  void SequencedValuePublisher<QueryType, ValueType>::Push(
      ValueForward&& value) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(m_queryId == -1) {
      m_writeLog.emplace_back(std::forward<ValueForward>(value));
    } else {
      if(value.GetSequence() >= m_nextSequence &&
          TestFilter(*m_filter, *value)) {
        m_nextSequence = Increment(value.GetSequence());
        m_queue.Push(std::forward<ValueForward>(value));
      }
    }
  }

  template<typename QueryType, typename ValueType>
  template<typename ForwardIterator>
  void SequencedValuePublisher<QueryType, ValueType>::PushSnapshot(
      ForwardIterator begin, ForwardIterator end) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    for(auto& value : boost::iterator_range<ForwardIterator>(begin, end)) {
      if(value.GetSequence() >= m_nextSequence &&
          TestFilter(*m_filter, *value)) {
        m_nextSequence = Increment(value.GetSequence());
        m_queue.Push(std::move(value));
      }
    }
  }

  template<typename QueryType, typename ValueType>
  void SequencedValuePublisher<QueryType, ValueType>::Break() {
    m_queue.Break();
  }

  template<typename QueryType, typename ValueType>
  void SequencedValuePublisher<QueryType, ValueType>::BeginSnapshot() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_queryId = -1;
  }

  template<typename QueryType, typename ValueType>
  void SequencedValuePublisher<QueryType, ValueType>::EndSnapshot(int queryId) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_queryId = queryId;
    std::vector<Value> writeLog;
    writeLog.swap(m_writeLog);
    for(const auto& value : writeLog) {
      if(value.GetSequence() >= m_nextSequence &&
          TestFilter(*m_filter, *value)) {
        m_nextSequence = Increment(value.GetSequence());
        m_queue.Push(std::move(value));
      }
    }
  }

  template<typename QueryType, typename ValueType>
  typename SequencedValuePublisher<QueryType, ValueType>::Query
      SequencedValuePublisher<QueryType, ValueType>::BeginRecovery() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_queryId = -1;
    if(m_query.GetInterruptionPolicy() == InterruptionPolicy::RECOVER_DATA ||
        m_query.GetInterruptionPolicy() ==
        InterruptionPolicy::IGNORE_CONTINUE) {
      Range::Point startPoint;
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

  template<typename QueryType, typename ValueType>
  void SequencedValuePublisher<QueryType, ValueType>::EndRecovery(int queryId) {
    EndSnapshot(queryId);
  }
}
}

#endif
