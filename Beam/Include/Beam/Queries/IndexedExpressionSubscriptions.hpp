#ifndef BEAM_INDEXED_EXPRESSION_SUBSCRIPTIONS_HPP
#define BEAM_INDEXED_EXPRESSION_SUBSCRIPTIONS_HPP
#include <boost/functional/factory.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/ExpressionSubscriptions.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {

  /**
   * Keeps track of streaming subscriptions to expression based IndexQueries.
   * @tparam T The type of data being input to the expression.
   * @tparam O The type of data being output by the expression.
   * @tparam I The type of index queried.
   * @tparam C The type of ServiceProtocolClients subscribing to queries.
   */
  template<typename T, typename O, typename I, typename C>
  class IndexedExpressionSubscriptions {
    public:

      /** The type of data being input to the expression. */
      using Input = T;

      /** The type of data being output by the expression. */
      using Output = O;

      /** The type of index. */
      using Index = I;

      /** The type of ServiceProtocolClients subscribing to queries. */
      using ServiceProtocolClient = C;

      /** Constructs an IndexedExpressionSubscriptions object. */
      IndexedExpressionSubscriptions() = default;

      /**
       * Initializes an expression based subscription.
       * @param index The subscription's index.
       * @param client The client initializing the subscription.
       * @param id The id used by the client to identify this query.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @param update_policy Specifies when updates should be published.
       * @param expression The expression to apply to the query.
       */
      void init(const Index& index, ServiceProtocolClient& client, int id,
        const Range& range, std::unique_ptr<Evaluator> filter,
        ExpressionQuery::UpdatePolicy update_policy,
        std::unique_ptr<Evaluator> expression);

      /**
       * Commits a previously initialized subscription.
       * @param index The subscription's index.
       * @param client The client committing the subscription.
       * @param snapshot_limit The limits used when calculating the snapshot.
       * @param result The result of the query.
       * @param snapshot The snapshot used.
       * @param f The function to call with the result of the query.
       */
      template<typename F>
      void commit(const Index& index, const ServiceProtocolClient& client,
        const SnapshotLimit& snapshot_limit,
        QueryResult<SequencedValue<Output>> result,
        std::vector<SequencedValue<Input>> snapshot, F&& f);

      /**
       * Ends a subscription.
       * @param client The client ending the subscription.
       * @param id The query's id.
       */
      void end(const ServiceProtocolClient& client, int id);

      /**
       * Removes all of a client's subscriptions.
       * @param client The client whose subscriptions are to be removed.
       */
      void remove_all(ServiceProtocolClient& client);

      /**
       * Publishes a value to all clients who subscribed to it.
       * @param value The value to publish.
       * @param sender The function called to send the value to the
       *        ServiceProtocolClient.
       */
      template<typename Sender>
      void publish(const SequencedValue<IndexedValue<Input, Index>>& value,
        const Sender& sender);

    private:
      using BaseSubscriptions =
        ExpressionSubscriptions<Input, Output, ServiceProtocolClient>;
      SynchronizedUnorderedMap<Index, std::shared_ptr<BaseSubscriptions>>
        m_subscriptions;
      SynchronizedUnorderedMap<const ServiceProtocolClient*,
        SynchronizedUnorderedMap<int, Index>> m_indexes;

      IndexedExpressionSubscriptions(
        const IndexedExpressionSubscriptions&) = delete;
      IndexedExpressionSubscriptions& operator =(
        const IndexedExpressionSubscriptions&) = delete;
  };

  template<typename T, typename O, typename I, typename C>
  void IndexedExpressionSubscriptions<T, O, I, C>::init(
      const Index& index, ServiceProtocolClient& client, int id,
      const Range& range, std::unique_ptr<Evaluator> filter,
      ExpressionQuery::UpdatePolicy update_policy,
      std::unique_ptr<Evaluator> expression) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    m_indexes.get(&client).insert(id, index);
    subscriptions.init(client, id, range, std::move(filter), update_policy,
      std::move(expression));
  }

  template<typename T, typename O, typename I, typename C>
  template<typename F>
  void IndexedExpressionSubscriptions<T, O, I, C>::commit(const Index& index,
      const ServiceProtocolClient& client, const SnapshotLimit& snapshot_limit,
      QueryResult<SequencedValue<Output>> result,
      std::vector<SequencedValue<Input>> snapshot, F&& f) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.commit(client, snapshot_limit, std::move(result),
      std::move(snapshot), std::forward<F>(f));
  }

  template<typename T, typename O, typename I, typename C>
  void IndexedExpressionSubscriptions<T, O, I, C>::end(
      const ServiceProtocolClient& client, int id) {
    auto indexes = m_indexes.find(&client);
    if(!indexes) {
      return;
    }
    auto index = indexes->find(id);
    if(!index) {
      return;
    }
    auto& subscriptions = *m_subscriptions.get_or_insert(
      *index, boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.end(client, id);
  }

  template<typename T, typename O, typename I, typename C>
  void IndexedExpressionSubscriptions<T, O, I, C>::remove_all(
      ServiceProtocolClient& client) {
    m_indexes.erase(&client);
    m_subscriptions.for_each_value([&] (auto& subscription) {
      subscription->remove_all(client);
    });
  }

  template<typename T, typename O, typename I, typename C>
  template<typename Sender>
  void IndexedExpressionSubscriptions<T, O, I, C>::publish(
      const SequencedValue<IndexedValue<Input, Index>>& value,
      const Sender& sender) {
    auto& subscriptions = *m_subscriptions.get_or_insert(
      value->get_index(), boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.publish(value, sender);
  }
}

#endif
