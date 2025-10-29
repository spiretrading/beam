#ifndef BEAM_QUERY_CLIENT_PUBLISHER_HPP
#define BEAM_QUERY_CLIENT_PUBLISHER_HPP
#include <exception>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <boost/range/adaptor/map.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SequencedValuePublisher.hpp"
#include "Beam/Queues/ConverterQueueWriter.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"

namespace Beam {

  /**
   * Used by clients to submit and update queries.
   * @tparam V The type of value being queried.
   * @tparam Q The type of query to submit.
   * @tparam E The type of EvaluatorTranslator used.
   * @tparam C Used to access ServiceProtocolClients to submit queries to.
   * @tparam S The type of service used to submit a query.
   * @tparam M The type used to end a previous query.
   */
  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  class QueryClientPublisher {
    public:

      /** The type of value being queried. */
      using Value = V;

      /** The type of query to submit. */
      using Query = Q;

      /** The query's index. */
      using Index = typename Query::Index;

      /** The type of EvaluatorTranslator used. */
      using EvaluatorTranslator = E;

      /** Used to access ServiceProtocolClients to submit queries to. */
      using ServiceProtocolClientHandler = C;

      /** The type of service used to submit a query. */
      using QueryService = S;

      /** The type used to end a previous query. */
      using EndQueryMessage = M;

      /** The type of ServiceProtocolClient used to submit queries to. */
      using ServiceProtocolClient =
        typename ServiceProtocolClientHandler::Client;

      /**
       * Constructs a QueryClientPublisher.
       * @param client_handler The ServiceProtocolClientHandler providing
       *        ServiceProtocolClients to submit queries to.
       */
      explicit QueryClientPublisher(
        Ref<ServiceProtocolClientHandler> client_handler);

      /**
       * Submits a query.
       * @param query The query to submit.
       * @param queue The QueueWriter receiving the result of the query.
       */
      void submit(
        const Query& query, ScopedQueueWriter<SequencedValue<Value>> queue);

      /**
       * Submits a query.
       * @param query The query to submit.
       * @param queue The QueueWriter receiving the result of the query.
       */
      void submit(const Query& query, ScopedQueueWriter<Value> queue);

      /**
       * Publishes a value to all subscribers.
       * @param value The value to publish.
       */
      void publish(const SequencedValue<IndexedValue<Value, Index>>& value);

      /**
       * Recovers all queries.
       * @param client The client to submit the recovery to.
       */
      void recover(ServiceProtocolClient& client);

      /**
       * Adds a message handler to automatically publish values when received
       * from a ServiceProtocolClient.
       * @tparam QueryMessage The type of Service message used to send query
       *         updates.
       */
      template<typename QueryMessage>
      void add_message_handler();

      /** Breaks all queues, ending all real time queries. */
      void close();

    private:
      using Publisher = SequencedValuePublisher<Query, Value>;
      using PublisherList = SynchronizedVector<std::shared_ptr<Publisher>>;
      using Publishers = std::unordered_map<Index, PublisherList>;
      ServiceProtocolClientHandler* m_client_handler;
      SynchronizedMap<Publishers> m_publishers;
      Sync<std::exception_ptr> m_break_exception;
      RoutineHandlerGroup m_query_routines;

      QueryClientPublisher(const QueryClientPublisher&) = delete;
      QueryClientPublisher& operator =(const QueryClientPublisher&) = delete;
  };

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  QueryClientPublisher<V, Q, E, C, S, M>::QueryClientPublisher(
    Ref<ServiceProtocolClientHandler> client_handler)
    : m_client_handler(client_handler.get()) {}

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::submit(
      const Query& query, ScopedQueueWriter<SequencedValue<Value>> queue) {
    if(query.get_range().get_end() == Sequence::LAST) {
      m_query_routines.spawn([=, this, queue = std::move(queue)] () mutable {
        auto filter = translate<EvaluatorTranslator>(query.get_filter());
        auto publisher = std::make_shared<Publisher>(
          query, std::move(filter), std::move(queue));
        auto& publisher_list = m_publishers.get(query.get_index());
        try {
          publisher->begin_snapshot();
          publisher_list.push_back(publisher);
          auto send_request = m_break_exception.with(
            [&] (auto& break_exception) {
              if(break_exception) {
                publisher->close(break_exception);
                return false;
              }
              return true;
            });
          if(send_request) {
            auto client = m_client_handler->get_client();
            auto result = client->template send_request<QueryService>(query);
            publisher->push_snapshot(
              result.m_snapshot.begin(), result.m_snapshot.end());
            publisher->end_snapshot(result.m_id);
          }
        } catch(const std::exception&) {
          publisher->close();
          publisher_list.erase(publisher);
        }
      });
    } else {
      m_query_routines.spawn([=, this, queue = std::move(queue)] () mutable {
        try {
          auto client = m_client_handler->get_client();
          auto result = client->template send_request<QueryService>(query);
          for(auto& value : result.m_snapshot) {
            queue.push(std::move(value));
          }
        } catch(const std::exception&) {}
        queue.close();
      });
    }
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::submit(const Query& query,
      ScopedQueueWriter<Value> queue) {
    auto conversion_queue = convert<SequencedValue<Value>>(
      std::move(queue), [] (auto&& value) {
        return static_cast<Value>(std::forward<decltype(value)>(value));
      });
    submit(query, std::move(conversion_queue));
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::publish(
      const SequencedValue<IndexedValue<Value, Index>>& value) {
    auto& publishers = m_publishers.get(value->get_index());
    auto client = m_client_handler->get_client();
    publishers.erase_if([&] (auto& publisher) {
      try {
        publisher->push(value);
        return false;
      } catch(const std::exception&) {
        if(publisher->get_id() != -1) {
          send_record_message<EndQueryMessage>(
            *client, value->get_index(), publisher->get_id());
          return true;
        } else {
          return false;
        }
      }
    });
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::recover(
      ServiceProtocolClient& client) {
    auto disconnected_publishers =
      std::vector<std::tuple<PublisherList*, std::shared_ptr<Publisher>>>();
    m_publishers.with([&] (auto& publishers) {
      for(auto& publisher_list : publishers | boost::adaptors::map_values) {
        publisher_list.for_each([&] (const auto& publisher) {
          if(publisher->get_id() != -1) {
            disconnected_publishers.push_back(
              std::tuple(&publisher_list, publisher));
          }
        });
      }
    });
    for(auto& disconnected_publisher : disconnected_publishers) {
      auto& publisher_list = *std::get<0>(disconnected_publisher);
      auto& publisher = std::get<1>(disconnected_publisher);
      try {
        auto query = publisher->begin_recovery();
        auto result = client.template send_request<QueryService>(query);
        publisher->push_snapshot(
          result.m_snapshot.begin(), result.m_snapshot.end());
        publisher->end_recovery(result.m_id);
      } catch(const std::exception&) {
        publisher->close();
        publisher_list.erase(publisher);
      }
    }
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  template<typename QueryMessage>
  void QueryClientPublisher<V, Q, E, C, S, M>::add_message_handler() {
    add_message_slot<QueryMessage>(out(m_client_handler->get_slots()),
      [this] (auto& sender, const auto& value) {
        publish(value);
      });
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::close() {
    auto break_exception = m_break_exception.with([&] (auto& break_exception) {
      if(break_exception) {
        return std::exception_ptr();
      }
      break_exception = std::make_exception_ptr(PipeBrokenException());
      return break_exception;
    });
    if(!break_exception) {
      return;
    }
    m_publishers.with([&] (auto& publishers) {
      for(auto& publisher : publishers | boost::adaptors::map_values) {
        publisher.for_each([&] (auto& disconnected_publisher) {
          disconnected_publisher->close(break_exception);
        });
      }
    });
  }
}

#endif
