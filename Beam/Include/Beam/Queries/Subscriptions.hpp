#ifndef BEAM_QUERY_SUBSCRIPTIONS_HPP
#define BEAM_QUERY_SUBSCRIPTIONS_HPP
#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam::Queries {

  /**
   * Keeps track of subscriptions to data streamed via a query.
   * @param <V> The type of data published.
   * @param <C> The type of ServiceProtocolClients subscribing to queries.
   */
  template<typename V, typename C>
  class Subscriptions {
    public:

      /** The type of data published. */
      using Value = SequencedValue<V>;

      /** The type of ServiceProtocolClients subscribing to queries. */
      using ServiceProtocolClient = C;

      /** Constructs a Subscriptions object. */
      Subscriptions();

      /**
       * Adds a subscription combining the initialization and commit.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int Add(ServiceProtocolClient& client, const Range& range,
        std::unique_ptr<Evaluator> filter);

      /**
       * Initializes a subscription.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int Initialize(ServiceProtocolClient& client, const Range& range,
        std::unique_ptr<Evaluator> filter);

      /**
       * Commits a previously initialized subscription.
       * @param result The result of the query.
       */
      template<typename F>
      void Commit(QueryResult<Value> result, F&& f);

      /**
       * Ends a subscription.
       * @param id The query's id.
       */
      void End(int id);

      /**
       * Removes all of a client's subscriptions.
       * @param client The client whose subscriptions are to be removed.
       */
      void RemoveAll(ServiceProtocolClient& client);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param clientFilter The function called to filter out clients to send
       *        the value to.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename ClientFilter, typename Sender>
      void Publish(const Value& value, ClientFilter&& clientFilter,
        Sender&& sender);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename Sender>
      void Publish(const Value& value, Sender&& sender);

    private:
      struct SubscriptionEntry {
        enum class State {
          INITIALIZING,
          COMMITTED
        };
        State m_state;
        int m_id;
        ServiceProtocolClient* m_client;
        Range m_range;
        std::unique_ptr<Evaluator> m_filter;
        std::vector<Value> m_writeLog;
        std::mutex m_mutex;

        SubscriptionEntry(int id, ServiceProtocolClient& client,
          const Range& range, std::unique_ptr<Evaluator> filter);
      };
      std::atomic_int m_nextQueryId;
      SynchronizedVector<std::shared_ptr<SubscriptionEntry>> m_subscriptions;
      std::vector<ServiceProtocolClient*> m_receivingClients;
      Beam::SynchronizedUnorderedMap<int, std::shared_ptr<SubscriptionEntry>>
        m_initializingSubscriptions;

      Subscriptions(const Subscriptions&) = delete;
      Subscriptions& operator =(const Subscriptions&) = delete;
  };

  template<typename V, typename C>
  Subscriptions<V, C>::SubscriptionEntry::SubscriptionEntry(int id,
    ServiceProtocolClient& client, const Range& range,
    std::unique_ptr<Evaluator> filter)
    : m_state(State::INITIALIZING),
      m_id(id),
      m_client(&client),
      m_range(range),
      m_filter(std::move(filter)) {}

  template<typename V, typename C>
  Subscriptions<V, C>::Subscriptions()
    : m_nextQueryId(0) {}

  template<typename V, typename C>
  int Subscriptions<V, C>::Add(ServiceProtocolClient& client,
      const Range& range, std::unique_ptr<Evaluator> filter) {
    auto queryId = Initialize(client, range, std::move(filter));
    auto result = QueryResult<Value>();
    result.m_queryId = queryId;
    Commit(std::move(result), [] (const QueryResult<Value>&) {});
    return queryId;
  }

  template<typename V, typename C>
  int Subscriptions<V, C>::Initialize(ServiceProtocolClient& client,
      const Range& range, std::unique_ptr<Evaluator> filter) {
    if(range.GetEnd() != Beam::Queries::Sequence::Last()) {
      return -1;
    }
    auto queryId = ++m_nextQueryId;
    auto subscriptionEntry = std::make_shared<SubscriptionEntry>(queryId,
      client, range, std::move(filter));
    m_initializingSubscriptions.Insert(queryId, subscriptionEntry);
    m_subscriptions.With([&] (auto& subscriptionList) {
      auto insertIterator = std::lower_bound(subscriptionList.begin(),
        subscriptionList.end(), subscriptionEntry,
        [] (const auto& lhs, const auto& rhs) {
          return lhs->m_client < rhs->m_client;
        });
      subscriptionList.insert(insertIterator, subscriptionEntry);
    });
    return queryId;
  }

  template<typename V, typename C>
  template<typename F>
  void Subscriptions<V, C>::Commit(QueryResult<Value> result, F&& f) {
    if(result.m_queryId == -1) {
      std::forward<F>(f)(std::move(result));
      return;
    }
    auto subscriptionEntryLookup = m_initializingSubscriptions.FindValue(
      result.m_queryId);
    if(!subscriptionEntryLookup.is_initialized()) {
      return;
    }
    auto& subscriptionEntry = **subscriptionEntryLookup;
    m_initializingSubscriptions.Erase(result.m_queryId);
    auto lock = std::lock_guard(subscriptionEntry.m_mutex);
    if(result.m_snapshot.empty()) {
      result.m_snapshot.swap(subscriptionEntry.m_writeLog);
    } else {
      auto mergeIterator = std::find_if(subscriptionEntry.m_writeLog.begin(),
        subscriptionEntry.m_writeLog.end(),
        [&] (const Value& value) {
          return value.GetSequence() > result.m_snapshot.back().GetSequence();
        });
      result.m_snapshot.insert(result.m_snapshot.end(), mergeIterator,
        subscriptionEntry.m_writeLog.end());
      subscriptionEntry.m_writeLog = {};
    }
    subscriptionEntry.m_state = SubscriptionEntry::State::COMMITTED;
    std::forward<F>(f)(std::move(result));
  }

  template<typename V, typename C>
  void Subscriptions<V, C>::End(int id) {
    m_subscriptions.RemoveIf([&] (const auto& entry) {
      return entry->m_id == id;
    });
  }

  template<typename V, typename C>
  void Subscriptions<V, C>::RemoveAll(ServiceProtocolClient& client) {
    m_subscriptions.RemoveIf([&] (const auto& entry) {
      return entry->m_client == &client;
    });
  }

  template<typename V, typename C>
  template<typename ClientFilter, typename Sender>
  void Subscriptions<V, C>::Publish(const Value& value,
      ClientFilter&& clientFilter, Sender&& sender) {
    auto lastClient = static_cast<const ServiceProtocolClient*>(nullptr);
    m_subscriptions.With([&] (const auto& subscriptionEntries) {
      m_receivingClients.clear();
      for(auto& subscriptionEntry : subscriptionEntries) {
        if(subscriptionEntry->m_client == lastClient) {
          continue;
        }
        if(!std::forward<ClientFilter>(clientFilter)(
            *subscriptionEntry->m_client)) {
          lastClient = subscriptionEntry->m_client;
          continue;
        }
        auto lock = std::lock_guard(subscriptionEntry->m_mutex);
        if((subscriptionEntry->m_range.GetStart() == Sequence::Present() ||
            RangePointGreaterOrEqual(value,
            subscriptionEntry->m_range.GetStart())) &&
            RangePointLesserOrEqual(value,
            subscriptionEntry->m_range.GetEnd()) &&
            TestFilter(*subscriptionEntry->m_filter, *value)) {
          lastClient = subscriptionEntry->m_client;
          if(subscriptionEntry->m_state ==
              SubscriptionEntry::State::INITIALIZING) {
            subscriptionEntry->m_writeLog.push_back(value);
          } else {
            m_receivingClients.push_back(subscriptionEntry->m_client);
          }
        }
      }
      if(!m_receivingClients.empty()) {
        std::forward<Sender>(sender)(m_receivingClients);
      }
    });
  }

  template<typename V, typename C>
  template<typename Sender>
  void Subscriptions<V, C>::Publish(const Value& value, Sender&& sender) {
    Publish(value, [] (ServiceProtocolClient&) { return true; },
      std::forward<Sender>(sender));
  }
}

#endif
