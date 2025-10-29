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
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /**
   * Publishes SequencedValues sent via a query.
   * @tparam Q The type of query submitted.
   * @tparam V The type of data received.
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
      SequencedValuePublisher(Query query, std::unique_ptr<Evaluator> filter,
        ScopedQueueWriter<Value> queue);

      /** Returns the id of the query. */
      int get_id() const;

      /**
       * Publishes data.
       * @param value The value to publish.
       */
      template<
        std::convertible_to<typename SequencedValuePublisher<Q, V>::Value> T>
      void push(T&& value);

      /**
       * Publishes data used as part of a query's snapshot.
       * @param begin An iterator to the beginning of the snapshot.
       * @param end An iterator to one past the end of the snapshot.
       */
      template<std::input_iterator I>
      void push_snapshot(I begin, I end);

      /** Begins receiving data used to initialize the query. */
      void begin_snapshot();

      /**
       * Ends the snapshot.
       * @param id The query's id.
       */
      void end_snapshot(int id);

      /**
       * Begins recovering data lost from an interrupted query.
       * @return The query to submit to recover the lost data.
       */
      Query begin_recovery();

      /**
       * Ends the data recovery, resuming normal query publication.
       * @param id The resumed query's id.
       */
      void end_recovery(int id);

      /** Breaks the queue receiving the published data. */
      void close();

      /**
       * Breaks the queue receiving the published data.
       * @param e The exception used to break the queue.
       */
      template<typename E>
      void close(const E& e);

      /**
       * Breaks the queue receiving the published data.
       * @param e The exception used to break the queue.
       */
      void close(const std::exception_ptr& e);

    private:
      mutable boost::mutex m_mutex;
      Query m_query;
      std::unique_ptr<Evaluator> m_filter;
      ScopedQueueWriter<Value> m_queue;
      int m_id;
      Sequence m_next_sequence;
      std::vector<Value> m_write_log;

      SequencedValuePublisher(const SequencedValuePublisher&) = delete;
      SequencedValuePublisher& operator =(
        const SequencedValuePublisher&) = delete;
  };

  template<typename Q, typename V>
  SequencedValuePublisher<Q, V>::SequencedValuePublisher(Query query,
    std::unique_ptr<Evaluator> filter, ScopedQueueWriter<Value> queue)
    : m_query(std::move(query)),
      m_filter(std::move(filter)),
      m_queue(std::move(queue)),
      m_id(-1),
      m_next_sequence(Sequence::FIRST) {}

  template<typename Q, typename V>
  int SequencedValuePublisher<Q, V>::get_id() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_id;
  }

  template<typename Q, typename V>
  template<std::convertible_to<typename SequencedValuePublisher<Q, V>::Value> T>
  void SequencedValuePublisher<Q, V>::push(T&& value) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_id == -1) {
      m_write_log.emplace_back(std::forward<T>(value));
    } else if(value.get_sequence() >= m_next_sequence &&
        test_filter(*m_filter, *value)) {
      m_next_sequence = increment(value.get_sequence());
      m_queue.push(std::forward<T>(value));
    }
  }

  template<typename Q, typename V>
  template<std::input_iterator I>
  void SequencedValuePublisher<Q, V>::push_snapshot(I begin, I end) {
    auto lock = boost::lock_guard(m_mutex);
    for(auto& value : boost::iterator_range<I>(begin, end)) {
      if(value.get_sequence() >= m_next_sequence &&
          test_filter(*m_filter, *value)) {
        m_next_sequence = increment(value.get_sequence());
        m_queue.push(std::move(value));
      }
    }
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::begin_snapshot() {
    auto lock = boost::lock_guard(m_mutex);
    m_id = -1;
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::end_snapshot(int id) {
    auto lock = boost::lock_guard(m_mutex);
    m_id = id;
    auto write_log = std::vector<Value>();
    write_log.swap(m_write_log);
    for(auto& value : write_log) {
      if(value.get_sequence() >= m_next_sequence &&
          test_filter(*m_filter, *value)) {
        m_next_sequence = increment(value.get_sequence());
        m_queue.push(std::move(value));
      }
    }
  }

  template<typename Q, typename V>
  typename SequencedValuePublisher<Q, V>::Query
      SequencedValuePublisher<Q, V>::begin_recovery() {
    auto lock = boost::lock_guard(m_mutex);
    m_id = -1;
    if(m_query.get_interruption_policy() == InterruptionPolicy::RECOVER_DATA ||
        m_query.get_interruption_policy() ==
          InterruptionPolicy::IGNORE_CONTINUE) {
      auto start = [&] () -> Range::Point {
        if(m_query.get_interruption_policy() ==
            InterruptionPolicy::IGNORE_CONTINUE) {
          return Sequence::PRESENT;
        } else if(m_next_sequence == Sequence::FIRST) {
          return m_query.get_range().get_start();
        }
        return m_next_sequence;
      }();
      auto recovery_query = m_query;
      recovery_query.set_range(start, m_query.get_range().get_end());
      return recovery_query;
    } else {
      m_queue.close(QueryInterruptedException());
      boost::throw_with_location(QueryInterruptedException());
    }
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::end_recovery(int id) {
    end_snapshot(id);
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::close() {
    m_queue.close();
  }

  template<typename Q, typename V>
  template<typename E>
  void SequencedValuePublisher<Q, V>::close(const E& e) {
    close(std::make_exception_ptr(e));
  }

  template<typename Q, typename V>
  void SequencedValuePublisher<Q, V>::close(const std::exception_ptr& e) {
    m_queue.close(e);
  }
}

#endif
