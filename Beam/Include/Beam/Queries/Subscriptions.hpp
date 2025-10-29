#ifndef BEAM_QUERY_SUBSCRIPTIONS_HPP
#define BEAM_QUERY_SUBSCRIPTIONS_HPP
#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {

  /**
   * Keeps track of subscriptions to data streamed via a query.
   * @tparam V The type of data published.
   * @tparam C The type of ServiceProtocolClients subscribing to queries.
   */
  template<typename V, typename C>
  class Subscriptions {
    public:

      /** The type of data published. */
      using Value = SequencedValue<V>;

      /** The type of ServiceProtocolClients subscribing to queries. */
      using ServiceProtocolClient = C;

      /** Constructs a Subscriptions object. */
      Subscriptions() noexcept;

      /**
       * Adds a subscription combining the initialization and commit.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int add(ServiceProtocolClient& client, const Range& range,
        std::unique_ptr<Evaluator> filter);

      /**
       * Initializes a subscription.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int init(ServiceProtocolClient& client, const Range& range,
        std::unique_ptr<Evaluator> filter);

      /**
       * Commits a previously initialized subscription.
       * @param result The result of the query.
       */
      template<typename F>
      void commit(QueryResult<Value> result, F&& f);

      /**
       * Ends a subscription.
       * @param id The query's id.
       */
      void end(int id);

      /**
       * Removes all of a client's subscriptions.
       * @param client The client whose subscriptions are to be removed.
       */
      void remove_all(ServiceProtocolClient& client);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param client_filter The function called to filter out clients to send
       *        the value to.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename ClientFilter, typename Sender>
      void publish(
        const Value& value, ClientFilter client_filter, Sender&& sender);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename Sender>
      void publish(const Value& value, Sender&& sender);

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
        std::vector<Value> m_write_log;
        boost::mutex m_mutex;

        SubscriptionEntry(int id, ServiceProtocolClient& client,
          const Range& range, std::unique_ptr<Evaluator> filter);
      };
      std::atomic_int m_next_query_id;
      SynchronizedVector<std::shared_ptr<SubscriptionEntry>> m_subscriptions;
      std::vector<ServiceProtocolClient*> m_receiving_clients;
      Beam::SynchronizedUnorderedMap<int, std::shared_ptr<SubscriptionEntry>>
        m_initializing_subscriptions;

      Subscriptions(const Subscriptions&) = delete;
      Subscriptions& operator =(const Subscriptions&) = delete;
  };

  template<typename V, typename C>
  Subscriptions<V, C>::SubscriptionEntry::SubscriptionEntry(
    int id, ServiceProtocolClient& client, const Range& range,
    std::unique_ptr<Evaluator> filter)
    : m_state(State::INITIALIZING),
      m_id(id),
      m_client(&client),
      m_range(range),
      m_filter(std::move(filter)) {}

  template<typename V, typename C>
  Subscriptions<V, C>::Subscriptions() noexcept
    : m_next_query_id(0) {}

  template<typename V, typename C>
  int Subscriptions<V, C>::add(ServiceProtocolClient& client,
      const Range& range, std::unique_ptr<Evaluator> filter) {
    auto id = init(client, range, std::move(filter));
    auto result = QueryResult<Value>();
    result.m_id = id;
    commit(std::move(result), [] (const QueryResult<Value>&) {});
    return id;
  }

  template<typename V, typename C>
  int Subscriptions<V, C>::init(ServiceProtocolClient& client,
      const Range& range, std::unique_ptr<Evaluator> filter) {
    if(range.get_end() != Beam::Sequence::LAST) {
      return -1;
    }
    auto id = ++m_next_query_id;
    auto entry =
      std::make_shared<SubscriptionEntry>(id, client, range, std::move(filter));
    m_initializing_subscriptions.insert(id, entry);
    m_subscriptions.with([&] (auto& subscriptions) {
      auto i =
        std::lower_bound(subscriptions.begin(), subscriptions.end(), entry,
          [] (const auto& lhs, const auto& rhs) {
            return lhs->m_client < rhs->m_client;
          });
      subscriptions.insert(i, entry);
    });
    return id;
  }

  template<typename V, typename C>
  template<typename F>
  void Subscriptions<V, C>::commit(QueryResult<Value> result, F&& f) {
    if(result.m_id == -1) {
      std::forward<F>(f)(std::move(result));
      return;
    }
    auto subscription = m_initializing_subscriptions.try_load(result.m_id);
    if(!subscription) {
      return;
    }
    auto& entry = **subscription;
    m_initializing_subscriptions.erase(result.m_id);
    auto lock = boost::lock_guard(entry.m_mutex);
    if(result.m_snapshot.empty()) {
      result.m_snapshot.swap(entry.m_write_log);
    } else {
      auto i = std::find_if(entry.m_write_log.begin(), entry.m_write_log.end(),
        [&] (const Value& value) {
          return value.get_sequence() > result.m_snapshot.back().get_sequence();
        });
      result.m_snapshot.insert(
        result.m_snapshot.end(), i, entry.m_write_log.end());
      entry.m_write_log = {};
    }
    entry.m_state = SubscriptionEntry::State::COMMITTED;
    std::forward<F>(f)(std::move(result));
  }

  template<typename V, typename C>
  void Subscriptions<V, C>::end(int id) {
    m_subscriptions.erase_if([&] (const auto& entry) {
      return entry->m_id == id;
    });
  }

  template<typename V, typename C>
  void Subscriptions<V, C>::remove_all(ServiceProtocolClient& client) {
    m_subscriptions.erase_if([&] (const auto& entry) {
      return entry->m_client == &client;
    });
  }

  template<typename V, typename C>
  template<typename ClientFilter, typename Sender>
  void Subscriptions<V, C>::publish(const Value& value,
      ClientFilter client_filter, Sender&& sender) {
    auto last_client = static_cast<const ServiceProtocolClient*>(nullptr);
    m_subscriptions.with([&] (const auto& subscriptions) {
      m_receiving_clients.clear();
      for(auto& entry : subscriptions) {
        if(entry->m_client == last_client) {
          continue;
        }
        if(!client_filter(*entry->m_client)) {
          last_client = entry->m_client;
          continue;
        }
        auto lock = boost::lock_guard(entry->m_mutex);
        if((entry->m_range.get_start() == Sequence::PRESENT ||
            range_point_greater_or_equal(value, entry->m_range.get_start())) &&
              range_point_lesser_or_equal(value, entry->m_range.get_end()) &&
                test_filter(*entry->m_filter, *value)) {
          last_client = entry->m_client;
          if(entry->m_state == SubscriptionEntry::State::INITIALIZING) {
            entry->m_write_log.push_back(value);
          } else {
            m_receiving_clients.push_back(entry->m_client);
          }
        }
      }
      if(!m_receiving_clients.empty()) {
        std::forward<Sender>(sender)(m_receiving_clients);
      }
    });
  }

  template<typename V, typename C>
  template<typename Sender>
  void Subscriptions<V, C>::publish(const Value& value, Sender&& sender) {
    publish(value, [] (ServiceProtocolClient&) { return true; },
      std::forward<Sender>(sender));
  }
}

#endif
