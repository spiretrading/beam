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
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/SequencedValuePublisher.hpp"
#include "Beam/Queues/ConverterQueueWriter.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"

namespace Beam::Queries {

  /**
   * Used by clients to submit and update queries.
   * @param <V> The type of value being queried.
   * @param <Q> The type of query to submit.
   * @param <E> The type of EvaluatorTranslator used.
   * @param <C> Used to access ServiceProtocolClients to submit queries to.
   * @param <S> The type of service used to submit a query.
   * @param <M> The type used to end a previous query.
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
       * @param clientHandler The ServiceProtocolClientHandler providing
       *        ServiceProtocolClients to submit queries to.
       */
      QueryClientPublisher(Ref<ServiceProtocolClientHandler> clientHandler);

      /**
       * Submits a query.
       * @param query The query to submit.
       * @param queue The QueueWriter receiving the result of the query.
       */
      void SubmitQuery(const Query& query,
        ScopedQueueWriter<SequencedValue<Value>> queue);

      /**
       * Submits a query.
       * @param query The query to submit.
       * @param queue The QueueWriter receiving the result of the query.
       */
      void SubmitQuery(const Query& query, ScopedQueueWriter<Value> queue);

      /**
       * Publishes a value to all subscribers.
       * @param value The value to publish.
       */
      void Publish(const SequencedValue<IndexedValue<Value, Index>>& value);

      /**
       * Recovers all queries.
       * @param client The client to submit the recovery to.
       */
      void Recover(ServiceProtocolClient& client);

      /**
       * Adds a message handler to automatically publish values when received
       * from a ServiceProtocolClient.
       * @param <QueryMessage> The type of Service message used to send query
       *         updates.
       */
      template<typename QueryMessage>
      void AddMessageHandler();

      /** Breaks all queues, ending all real time queries. */
      void Break();

    private:
      using Publisher = SequencedValuePublisher<Query, Value>;
      using PublisherList = SynchronizedVector<std::shared_ptr<Publisher>>;
      using Publishers = std::unordered_map<Index, PublisherList>;
      ServiceProtocolClientHandler* m_clientHandler;
      SynchronizedMap<Publishers> m_publishers;
      Threading::Sync<std::exception_ptr> m_breakException;
      Routines::RoutineHandlerGroup m_queryRoutines;

      QueryClientPublisher(const QueryClientPublisher&) = delete;
      QueryClientPublisher& operator =(const QueryClientPublisher&) = delete;
  };

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  QueryClientPublisher<V, Q, E, C, S, M>::QueryClientPublisher(
    Ref<ServiceProtocolClientHandler> clientHandler)
    : m_clientHandler(clientHandler.Get()) {}

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::SubmitQuery(const Query& query,
      ScopedQueueWriter<SequencedValue<Value>> queue) {
    if(query.GetRange().GetEnd() == Sequence::Last()) {
      m_queryRoutines.Spawn([=, queue = std::move(queue)] () mutable {
        auto filter = Translate<EvaluatorTranslator>(query.GetFilter());
        auto publisher = std::make_shared<Publisher>(query, std::move(filter),
          std::move(queue));
        auto& publisherList = m_publishers.Get(query.GetIndex());
        try {
          publisher->BeginSnapshot();
          publisherList.PushBack(publisher);
          auto sendRequest = m_breakException.With([&] (auto& breakException)) {
            if(breakException) {
              publisher->Break(breakException);
              return false;
            }
            return true;
          });
          if(sendRequest) {
            auto client = m_clientHandler->GetClient();
            auto queryResult = client->template SendRequest<QueryService>(
              query);
            publisher->PushSnapshot(queryResult.m_snapshot.begin(),
              queryResult.m_snapshot.end());
            publisher->EndSnapshot(queryResult.m_queryId);
          }
        } catch(const std::exception&) {
          publisher->Break();
          publisherList.Remove(publisher);
        }
      });
    } else {
      m_queryRoutines.Spawn([=, queue = std::move(queue)] () mutable {
        try {
          auto client = m_clientHandler->GetClient();
          auto queryResult = client->template SendRequest<QueryService>(
            query);
          for(auto& value : queryResult.m_snapshot) {
            queue.Push(std::move(value));
          }
        } catch(const std::exception&) {}
        queue.Break();
      });
    }
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::SubmitQuery(const Query& query,
      ScopedQueueWriter<Value> queue) {
    auto conversionQueue = MakeConverterQueueWriter<SequencedValue<Value>>(
      std::move(queue),
      [] (auto&& value) {
        return static_cast<Value>(std::forward<decltype(value)>(value));
      });
    SubmitQuery(query, std::move(conversionQueue));
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::Publish(
      const SequencedValue<IndexedValue<Value, Index>>& value) {
    auto& publishers = m_publishers.Get(value->GetIndex());
    auto client = m_clientHandler->GetClient();
    publishers.With([&] (auto& publishers) {
      publishers.erase(std::remove_if(publishers.begin(), publishers.end(),
        [&] (auto& publisher) {
          try {
            publisher->Push(value);
            return false;
          } catch(const std::exception&) {
            if(publisher->GetId() != -1) {
              Services::SendRecordMessage<EndQueryMessage>(*client,
                value->GetIndex(), publisher->GetId());
              return true;
            } else {
              return false;
            }
          }
        }), publishers.end());
    });
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::Recover(
      ServiceProtocolClient& client) {
    auto disconnectedPublishers =
      std::vector<std::tuple<PublisherList*, std::shared_ptr<Publisher>>>();
    m_publishers.With([&] (auto& publishers) {
      for(auto& publisherList : publishers | boost::adaptors::map_values) {
        publisherList.ForEach([&] (const auto& publisher) {
          if(publisher->GetId() != -1) {
            disconnectedPublishers.push_back(
              std::tuple(&publisherList, publisher));
          }
        });
      }
    });
    for(auto& publisherEntry : disconnectedPublishers) {
      auto& publisherList = *std::get<0>(publisherEntry);
      auto& publisher = std::get<1>(publisherEntry);
      try {
        auto query = publisher->BeginRecovery();
        auto queryResult = client.template SendRequest<QueryService>(query);
        publisher->PushSnapshot(queryResult.m_snapshot.begin(),
          queryResult.m_snapshot.end());
        publisher->EndRecovery(queryResult.m_queryId);
      } catch(const std::exception&) {
        publisher->Break();
        publisherList.Remove(publisher);
      }
    }
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  template<typename QueryMessage>
  void QueryClientPublisher<V, Q, E, C, S, M>::AddMessageHandler() {
    Services::AddMessageSlot<QueryMessage>(Store(m_clientHandler->GetSlots()),
      [=] (auto& sender, const auto& value) {
        Publish(value);
      });
  }

  template<typename V, typename Q, typename E, typename C, typename S,
    typename M>
  void QueryClientPublisher<V, Q, E, C, S, M>::Break() {
    auto breakException = m_breakException.With([&] (auto& breakException) {
      if(breakException) {
        return std::exception_ptr();
      }
      breakException = std::make_exception_ptr(PipeBrokenException());
      return breakException;
    });
    if(!breakException) {
      return;
    }
    m_publishers.With([&] (auto& publishers) {
      for(auto& publisher : publishers) {
        publisher->Break(breakException);
      }
    });
  }
}

#endif
