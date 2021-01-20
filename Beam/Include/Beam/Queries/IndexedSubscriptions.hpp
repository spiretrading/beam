#ifndef BEAM_INDEXED_QUERY_SUBSCRIPTIONS_HPP
#define BEAM_INDEXED_QUERY_SUBSCRIPTIONS_HPP
#include <boost/functional/factory.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/Subscriptions.hpp"

namespace Beam::Queries {

  /**
   * Keeps track of subscriptions to data streamed via an IndexedQuery.
   * @param <V> The type of data published.
   * @param <I> The type of index queried.
   * @param <C> The type of ServiceProtocolClients subscribing to queries.
   */
  template<typename V, typename I, typename C>
  class IndexedSubscriptions {
    public:

      /** The type of data published. */
      using Value = SequencedValue<IndexedValue<V, I>>;

      /** The type of the base value. */
      using BaseValue = typename Value::Value::Value;

      /** The type of index. */
      using Index = typename Value::Value::Index;

      /** The type of ServiceProtocolClients subscribing to queries. */
      using ServiceProtocolClient = C;

      /** Constructs an IndexedSubscriptions object. */
      IndexedSubscriptions() = default;

      /**
       * Adds a subscription combining the initialization and commit.
       * @param index The subscription's index.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int Add(const Index& index, ServiceProtocolClient& client,
        const Range& range, std::unique_ptr<Evaluator> filter);

      /**
       * Initializes a subscription.
       * @param index The subscription's index.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int Initialize(const Index& index, ServiceProtocolClient& client,
        const Range& range, std::unique_ptr<Evaluator> filter);

      /**
       * Commits a previously initialized subscription.
       * @param index The index of the subscription to commit.
       * @param result The result of the query.
       * @param f The function to call with the QueryResult synchronized to the
       *        subscription.
       */
      template<typename F>
      void Commit(const Index& index,
        QueryResult<SequencedValue<BaseValue>> result, F&& f);

      /**
       * Ends a subscription.
       * @param index The query's index.
       * @param id The query's id.
       */
      void End(const Index& index, int id);

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
      void Publish(const Value& value, const ClientFilter& clientFilter,
        const Sender& sender);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename Sender>
      void Publish(const Value& value, const Sender& sender);

    private:
      using BaseSubscriptions = Subscriptions<BaseValue, ServiceProtocolClient>;
      SynchronizedUnorderedMap<Index, std::shared_ptr<BaseSubscriptions>>
        m_subscriptions;

      IndexedSubscriptions(const IndexedSubscriptions&) = delete;
      IndexedSubscriptions& operator =(const IndexedSubscriptions&) = delete;
  };

  template<typename V, typename I, typename C>
  int IndexedSubscriptions<V, I, C>::Add(const Index& index,
      ServiceProtocolClient& client, const Range& range,
      std::unique_ptr<Evaluator> filter) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.Add(client, range, std::move(filter));
  }

  template<typename V, typename I, typename C>
  int IndexedSubscriptions<V, I, C>::Initialize(const Index& index,
      ServiceProtocolClient& client, const Range& range,
      std::unique_ptr<Evaluator> filter) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.Initialize(client, range, std::move(filter));
  }

  template<typename V, typename I, typename C>
  template<typename F>
  void IndexedSubscriptions<V, I, C>::Commit(const Index& index,
      QueryResult<SequencedValue<BaseValue>> result, F&& f) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.Commit(std::move(result), std::forward<F>(f));
  }

  template<typename V, typename I, typename C>
  void IndexedSubscriptions<V, I, C>::End(const Index& index, int id) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.End(id);
  }

  template<typename V, typename I, typename C>
  void IndexedSubscriptions<V, I, C>::RemoveAll(ServiceProtocolClient& client) {
    m_subscriptions.With([&] (auto& subscriptions) {
      for(auto& baseSubscription :
          subscriptions | boost::adaptors::map_values) {
        baseSubscription->RemoveAll(client);
      }
    });
  }

  template<typename V, typename I, typename C>
  template<typename ClientFilter, typename Sender>
  void IndexedSubscriptions<V, I, C>::Publish(const Value& value,
      const ClientFilter& clientFilter, const Sender& sender) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(
      value->GetIndex(), boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.Publish(value, clientFilter, sender);
  }

  template<typename V, typename I, typename C>
  template<typename Sender>
  void IndexedSubscriptions<V, I, C>::Publish(const Value& value,
      const Sender& sender) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(
      value->GetIndex(), boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.Publish(value, sender);
  }
}

#endif
