#ifndef BEAM_QUERYCLIENTPUBLISHER_HPP
#define BEAM_QUERYCLIENTPUBLISHER_HPP
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <boost/noncopyable.hpp>
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
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"

namespace Beam {
namespace Queries {

  /*! \class QueryClientPublisher
      \brief Used by clients to submit and update queries.
      \tparam ValueType The type of value being queried.
      \tparam QueryType The type of query to submit.
      \tparam EvaluatorTranslatorType The type of EvaluatorTranslator used.
      \tparam ServiceProtocolClientHandlerType Used to access
              ServiceProtocolClients to submit queries to.
      \tparam QueryServiceType The type of service used to submit a query.
      \tparam EndQueryMessageType The type used to end a previous query.
   */
  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  class QueryClientPublisher : private boost::noncopyable {
    public:

      //! The type of value being queried.
      using Value = ValueType;

      //! The type of query to submit.
      using Query = QueryType;

      //! The query's index.
      using Index = typename Query::Index;

      //! The type of EvaluatorTranslator used.
      using EvaluatorTranslator = EvaluatorTranslatorType;

      //! Used to access ServiceProtocolClients to submit queries to.
      using ServiceProtocolClientHandler = ServiceProtocolClientHandlerType;

      //! The type of service used to submit a query.
      using QueryService = QueryServiceType;

      //! The type used to end a previous query.
      using EndQueryMessage = EndQueryMessageType;

      //! The type of ServiceProtocolClient used to submit queries to.
      using ServiceProtocolClient =
        typename ServiceProtocolClientHandler::Client;

      //! Constructs a QueryClientPublisher.
      /*!
        \param clientHandler The ServiceProtocolClientHandler providing
               ServiceProtocolClients to submit queries to.
      */
      QueryClientPublisher(Ref<ServiceProtocolClientHandler> clientHandler);

      //! Submits a query.
      /*!
        \param query The query to submit.
        \param queue The QueueWriter receiving the result of the query.
      */
      void SubmitQuery(const Query& query,
        ScopedQueueWriter<SequencedValue<Value>> queue);

      //! Submits a query.
      /*!
        \param query The query to submit.
        \param queue The QueueWriter receiving the result of the query.
      */
      void SubmitQuery(const Query& query, ScopedQueueWriter<Value> queue);

      //! Publishes a value to all subscribers.
      /*!
        \param value The value to publish.
      */
      void Publish(const SequencedValue<IndexedValue<Value, Index>>& value);

      //! Recovers all queries.
      /*!
        \param client The client to submit the recovery to.
      */
      void Recover(ServiceProtocolClient& client);

      //! Adds a message handler to automatically publish values when received
      //! from a ServiceProtocolClient.
      /*!
        \tparam QueryMessage The type of Service message used to send query
                updates.
      */
      template<typename QueryMessage>
      void AddMessageHandler();

    private:
      using Publisher = SequencedValuePublisher<Query, Value>;
      using PublisherList = SynchronizedVector<std::shared_ptr<Publisher>>;
      using Publishers = std::unordered_map<Index, PublisherList>;
      ServiceProtocolClientHandler* m_clientHandler;
      SynchronizedMap<Publishers> m_publishers;
      Routines::RoutineHandlerGroup m_queryRoutines;
  };

  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  QueryClientPublisher<ValueType, QueryType, EvaluatorTranslatorType,
      ServiceProtocolClientHandlerType, QueryServiceType, EndQueryMessageType>::
      QueryClientPublisher(Ref<ServiceProtocolClientHandler> clientHandler)
      : m_clientHandler(clientHandler.Get()) {}

  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  void QueryClientPublisher<ValueType, QueryType, EvaluatorTranslatorType,
      ServiceProtocolClientHandlerType, QueryServiceType, EndQueryMessageType>::
      SubmitQuery(const Query& query,
      ScopedQueueWriter<SequencedValue<Value>> queue) {
    if(query.GetRange().GetEnd() == Sequence::Last()) {
      m_queryRoutines.Spawn(
        [=, queue = std::move(queue)] () mutable {
          auto filter = Translate<EvaluatorTranslator>(query.GetFilter());
          auto publisher = std::make_shared<Publisher>(query, std::move(filter),
            std::move(queue));
          auto& publisherList = m_publishers.Get(query.GetIndex());
          try {
            publisher->BeginSnapshot();
            publisherList.PushBack(publisher);
            auto client = m_clientHandler->GetClient();
            auto queryResult = client->template SendRequest<QueryService>(
              query);
            publisher->PushSnapshot(queryResult.m_snapshot.begin(),
              queryResult.m_snapshot.end());
            publisher->EndSnapshot(queryResult.m_queryId);
          } catch(const std::exception&) {
            publisher->Break();
            publisherList.Remove(publisher);
          }
        });
    } else {
      m_queryRoutines.Spawn(
        [=, queue = std::move(queue)] () mutable {
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

  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  void QueryClientPublisher<ValueType, QueryType, EvaluatorTranslatorType,
      ServiceProtocolClientHandlerType, QueryServiceType, EndQueryMessageType>::
      SubmitQuery(const Query& query, ScopedQueueWriter<Value> queue) {
    auto conversionQueue = MakeConverterQueueWriter<SequencedValue<Value>>(
      std::move(queue),
      [] (auto&& value) {
        return static_cast<Value>(std::forward<decltype(value)>(value));
      });
    SubmitQuery(query, std::move(conversionQueue));
  }

  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  void QueryClientPublisher<ValueType, QueryType, EvaluatorTranslatorType,
      ServiceProtocolClientHandlerType, QueryServiceType, EndQueryMessageType>::
      Publish(const SequencedValue<IndexedValue<Value, Index>>& value) {
    auto& publishers = m_publishers.Get(value->GetIndex());
    auto client = m_clientHandler->GetClient();
    publishers.With(
      [&] (std::vector<std::shared_ptr<Publisher>>& publishers) {
        auto i = publishers.begin();
        while(i != publishers.end()) {
          const auto& publisher = *i;
          try {
            publisher->Push(value);
            ++i;
          } catch(const std::exception&) {
            if(publisher->GetId() != -1) {
              Services::SendRecordMessage<EndQueryMessage>(*client,
                value->GetIndex(), publisher->GetId());
              i = publishers.erase(i);
            } else {
              ++i;
            }
          }
        }
      });
  }

  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  void QueryClientPublisher<ValueType, QueryType, EvaluatorTranslatorType,
      ServiceProtocolClientHandlerType, QueryServiceType, EndQueryMessageType>::
      Recover(ServiceProtocolClient& client) {
    std::vector<std::tuple<PublisherList*, std::shared_ptr<Publisher>>>
      disconnectedPublishers;
    m_publishers.With(
      [&] (Publishers& publishers) {
        for(auto& publisherList : publishers | boost::adaptors::map_values) {
          publisherList.ForEach(
            [&] (const std::shared_ptr<Publisher>& publisher) {
              if(publisher->GetId() != -1) {
                disconnectedPublishers.push_back(
                  std::make_tuple(&publisherList, publisher));
              }
            });
        }
      });
    for(const auto& publisherEntry : disconnectedPublishers) {
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

  template<typename ValueType, typename QueryType,
    typename EvaluatorTranslatorType, typename ServiceProtocolClientHandlerType,
    typename QueryServiceType, typename EndQueryMessageType>
  template<typename QueryMessage>
  void QueryClientPublisher<ValueType, QueryType, EvaluatorTranslatorType,
      ServiceProtocolClientHandlerType, QueryServiceType, EndQueryMessageType>::
      AddMessageHandler() {
    Services::AddMessageSlot<QueryMessage>(Store(m_clientHandler->GetSlots()),
      [=] (ServiceProtocolClient& sender,
          const SequencedValue<IndexedValue<Value, Index>>& value) {
        Publish(value);
      });
  }
}
}

#endif
