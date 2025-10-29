#ifndef BEAM_INDEXED_QUERY_SUBSCRIPTIONS_HPP
#define BEAM_INDEXED_QUERY_SUBSCRIPTIONS_HPP
#include <boost/functional/factory.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queries/Subscriptions.hpp"

namespace Beam {

  /**
   * Keeps track of subscriptions to data streamed via an IndexedQuery.
   * @tparam V The type of data published.
   * @tparam I The type of index queried.
   * @tparam C The type of ServiceProtocolClients subscribing to queries.
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
      int add(const Index& index, ServiceProtocolClient& client,
        const Range& range, std::unique_ptr<Evaluator> filter);

      /**
       * Initializes a subscription.
       * @param index The subscription's index.
       * @param client The client initializing the subscription.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @return The query's unique id.
       */
      int init(const Index& index, ServiceProtocolClient& client,
        const Range& range, std::unique_ptr<Evaluator> filter);

      /**
       * Commits a previously initialized subscription.
       * @param index The index of the subscription to commit.
       * @param result The result of the query.
       * @param f The function to call with the QueryResult synchronized to the
       *        subscription.
       */
      template<typename F>
      void commit(const Index& index,
        QueryResult<SequencedValue<BaseValue>> result, F&& f);

      /**
       * Ends a subscription.
       * @param index The query's index.
       * @param id The query's id.
       */
      void end(const Index& index, int id);

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
      void publish(const Value& value, const ClientFilter& client_filter,
        const Sender& sender);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename Sender>
      void publish(const Value& value, const Sender& sender);

    private:
      using BaseSubscriptions = Subscriptions<BaseValue, ServiceProtocolClient>;
      SynchronizedUnorderedMap<Index, std::shared_ptr<BaseSubscriptions>>
        m_subscriptions;

      IndexedSubscriptions(const IndexedSubscriptions&) = delete;
      IndexedSubscriptions& operator =(const IndexedSubscriptions&) = delete;
  };

  template<typename V, typename I, typename C>
  int IndexedSubscriptions<V, I, C>::add(
      const Index& index, ServiceProtocolClient& client, const Range& range,
      std::unique_ptr<Evaluator> filter) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.add(client, range, std::move(filter));
  }

  template<typename V, typename I, typename C>
  int IndexedSubscriptions<V, I, C>::init(const Index& index,
      ServiceProtocolClient& client, const Range& range,
      std::unique_ptr<Evaluator> filter) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.init(client, range, std::move(filter));
  }

  template<typename V, typename I, typename C>
  template<typename F>
  void IndexedSubscriptions<V, I, C>::commit(const Index& index,
      QueryResult<SequencedValue<BaseValue>> result, F&& f) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.commit(std::move(result), std::forward<F>(f));
  }

  template<typename V, typename I, typename C>
  void IndexedSubscriptions<V, I, C>::end(const Index& index, int id) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.end(id);
  }

  template<typename V, typename I, typename C>
  void IndexedSubscriptions<V, I, C>::remove_all(
      ServiceProtocolClient& client) {
    m_subscriptions.with([&] (auto& subscriptions) {
      for(auto& subscription : subscriptions | boost::adaptors::map_values) {
        subscription->remove_all(client);
      }
    });
  }

  template<typename V, typename I, typename C>
  template<typename ClientFilter, typename Sender>
  void IndexedSubscriptions<V, I, C>::publish(const Value& value,
      const ClientFilter& client_filter, const Sender& sender) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      value->get_index(), boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.publish(value, client_filter, sender);
  }

  template<typename V, typename I, typename C>
  template<typename Sender>
  void IndexedSubscriptions<V, I, C>::publish(
      const Value& value, const Sender& sender) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      value->get_index(), boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.publish(value, sender);
  }
}

#endif
