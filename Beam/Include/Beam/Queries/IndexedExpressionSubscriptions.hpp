#ifndef BEAM_INDEXEDEXPRESSIONSUBSCRIPTIONS_HPP
#define BEAM_INDEXEDEXPRESSIONSUBSCRIPTIONS_HPP
#include <boost/functional/factory.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/Queries/ExpressionSubscriptions.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SequencedValue.hpp"

namespace Beam {
namespace Queries {

  /*! \class IndexedExpressionSubscriptions
      \brief Keeps track of streaming subscriptions to expression based
             IndexQueries.
      \tparam ValueType The type of data published.
      \tparam IndexType The type of index queried.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClients
              subscribing to queries.
   */
  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType>
  class IndexedExpressionSubscriptions : private boost::noncopyable {
    public:

      //! The type of data being input to the expression.
      using Input = InputType;

      //! The type of data being output by the expression.
      using Output = OutputType;

      //! The type of index.
      using Index = IndexType;

      //! The type of ServiceProtocolClients subscribing to queries.
      using ServiceProtocolClient = ServiceProtocolClientType;

      //! Constructs an IndexedExpressionSubscriptions object.
      IndexedExpressionSubscriptions() = default;

      //! Initializes an expression based subscription.
      /*!
        \param index The subscription's index.
        \param client The client initializing the subscription.
        \param id The id used by the client to identify this query.
        \param range The Range of the query.
        \param filter The filter to apply to published values.
        \param updatePolicy Specifies when updates should be published.
        \param expression The expression to apply to the query.
      */
      void Initialize(const Index& index, ServiceProtocolClient& client, int id,
        const Range& range, std::unique_ptr<Evaluator> filter,
        ExpressionQuery::UpdatePolicy updatePolicy,
        std::unique_ptr<Evaluator> expression);

      //! Commits a previously initialized subscription.
      /*!
        \param index The subscription's index.
        \param client The client committing the subscription.
        \param snapshotLimit The limits used when calculating the snapshot.
        \param result The result of the query.
        \param snapshot The snapshot used.
        \param f The function to call with the result of the query.
      */
      template<typename F>
      void Commit(const Index& index, const ServiceProtocolClient& client,
        const SnapshotLimit& snapshotLimit,
        QueryResult<SequencedValue<Output>> result,
        std::vector<SequencedValue<Input>> snapshot, const F& f);

      //! Ends a subscription.
      /*!
        \param client The client ending the subscription.
        \param id The query's id.
      */
      void End(const ServiceProtocolClient& client, int id);

      //! Removes all of a client's subscriptions.
      /*!
        \param client The client whose subscriptions are to be removed.
      */
      void RemoveAll(ServiceProtocolClient& client);

      //! Publishes a value to all clients who subscribed to it.
      /*!
        \param value The value to publish.
        \param sender The function called to send the value to the
               ServiceProtocolClient.
      */
      template<typename Sender>
      void Publish(const SequencedValue<IndexedValue<Input, Index>>& value,
        const Sender& sender);

    private:
      using BaseSubscriptions = ExpressionSubscriptions<Input, Output,
        ServiceProtocolClient>;
      SynchronizedUnorderedMap<Index, std::shared_ptr<BaseSubscriptions>>
        m_subscriptions;
      SynchronizedUnorderedMap<const ServiceProtocolClient*,
        SynchronizedUnorderedMap<int, Index>> m_indexes;
  };

  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType>
  void IndexedExpressionSubscriptions<InputType, OutputType, IndexType,
      ServiceProtocolClientType>::Initialize(const Index& index,
      ServiceProtocolClient& client, int id, const Range& range,
      std::unique_ptr<Evaluator> filter,
      ExpressionQuery::UpdatePolicy updatePolicy,
      std::unique_ptr<Evaluator> expression) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(index,
      boost::factory<std::shared_ptr<BaseSubscriptions>>());
    m_indexes.Get(&client).Insert(id, index);
    subscriptions.Initialize(client, id, range, std::move(filter), updatePolicy,
      std::move(expression));
  }

  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType>
  template<typename F>
  void IndexedExpressionSubscriptions<InputType, OutputType, IndexType,
      ServiceProtocolClientType>::Commit(const Index& index,
      const ServiceProtocolClient& client, const SnapshotLimit& snapshotLimit,
      QueryResult<SequencedValue<Output>> result,
      std::vector<SequencedValue<Input>> snapshot, const F& f) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(index,
      boost::factory<std::shared_ptr<BaseSubscriptions>>());
    return subscriptions.Commit(client, snapshotLimit, std::move(result),
      std::move(snapshot), f);
  }

  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType>
  void IndexedExpressionSubscriptions<InputType, OutputType, IndexType,
      ServiceProtocolClientType>::End(const ServiceProtocolClient& client,
      int id) {
    auto indexes = m_indexes.Find(&client);
    if(!indexes.is_initialized()) {
      return;
    }
    auto index = indexes->Find(id);
    if(!index.is_initialized()) {
      return;
    }
    auto& subscriptions = *m_subscriptions.GetOrInsert(*index,
      boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.End(client, id);
  }

  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType>
  void IndexedExpressionSubscriptions<InputType, OutputType, IndexType,
      ServiceProtocolClientType>::RemoveAll(ServiceProtocolClient& client) {
    m_indexes.Erase(&client);
    m_subscriptions.With(
      [&] (std::unordered_map<Index, std::shared_ptr<BaseSubscriptions>>&
          subscriptions) {
        for(auto& baseSubscription :
            subscriptions | boost::adaptors::map_values) {
          baseSubscription->RemoveAll(client);
        }
      });
  }

  template<typename InputType, typename OutputType, typename IndexType,
    typename ServiceProtocolClientType>
  template<typename Sender>
  void IndexedExpressionSubscriptions<InputType, OutputType, IndexType,
      ServiceProtocolClientType>::Publish(
      const SequencedValue<IndexedValue<Input, Index>>& value,
      const Sender& sender) {
    auto& subscriptions = *m_subscriptions.GetOrInsert(value->GetIndex(),
      boost::factory<std::shared_ptr<BaseSubscriptions>>());
    subscriptions.Publish(value, sender);
  }
}
}

#endif
