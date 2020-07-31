#ifndef BEAM_EXPRESSIONSUBSCRIPTIONS_HPP
#define BEAM_EXPRESSIONSUBSCRIPTIONS_HPP
#include <memory>
#include <vector>
#include <boost/atomic/atomic.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/ExpressionQuery.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {
namespace Queries {

  /*! \class ExpressionSubscriptions
      \brief Keeps track of streaming subscriptions to expression based queries.
      \tparam InputType The type of data being input to the expression.
      \tparam OutputType The type of data being output by the expression.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClients
              subscribing to queries.
   */
  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  class ExpressionSubscriptions : private boost::noncopyable {
    public:

      //! The type of data being input to the expression.
      using Input = InputType;

      //! The type of data being output by the expression.
      using Output = OutputType;

      //! The type of ServiceProtocolClients subscribing to queries.
      using ServiceProtocolClient = ServiceProtocolClientType;

      //! Constructs an ExpressionSubscriptions object.
      ExpressionSubscriptions() = default;

      //! Initializes an expression based subscription.
      /*!
        \param client The client initializing the subscription.
        \param id The id used by the client to identify this query.
        \param range The Range of the query.
        \param filter The filter to apply to published values.
        \param updatePolicy Specifies when updates should be published.
        \param expression The expression to apply to the query.
      */
      void Initialize(ServiceProtocolClient& client, int id, const Range& range,
        std::unique_ptr<Evaluator> filter,
        ExpressionQuery::UpdatePolicy updatePolicy,
        std::unique_ptr<Evaluator> expression);

      //! Commits a previously initialized subscription.
      /*!
        \param client The client committing the subscription.
        \param snapshotLimit The limits used when calculating the snapshot.
        \param result The result of the query.
        \param snapshot The snapshot used.
        \param f The function to call with the result of the query.
      */
      template<typename F>
      void Commit(const ServiceProtocolClient& client,
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
      void Publish(const SequencedValue<Input>& value, const Sender& sender);

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
        ExpressionQuery::UpdatePolicy m_updatePolicy;
        std::unique_ptr<Evaluator> m_expression;
        boost::optional<Output> m_previousValue;
        std::vector<SequencedValue<Input>> m_writeLog;

        SubscriptionEntry(int id, ServiceProtocolClient& client,
          const Range& range, std::unique_ptr<Evaluator> filter,
          ExpressionQuery::UpdatePolicy updatePolicy,
          std::unique_ptr<Evaluator> expression);
      };
      using SyncSubscriptionEntry = Threading::Sync<SubscriptionEntry>;
      SynchronizedVector<std::shared_ptr<SyncSubscriptionEntry>>
        m_subscriptions;
      SynchronizedUnorderedMap<const ServiceProtocolClient*,
        SynchronizedUnorderedMap<int, std::shared_ptr<SyncSubscriptionEntry>>>
        m_initializingSubscriptions;
  };

  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  ExpressionSubscriptions<InputType, OutputType, ServiceProtocolClientType>::
      SubscriptionEntry::SubscriptionEntry(int id,
      ServiceProtocolClient& client, const Range& range,
      std::unique_ptr<Evaluator> filter,
      ExpressionQuery::UpdatePolicy updatePolicy,
      std::unique_ptr<Evaluator> expression)
      : m_state(State::INITIALIZING),
        m_id(id),
        m_client(&client),
        m_range(range),
        m_filter(std::move(filter)),
        m_updatePolicy(updatePolicy),
        m_expression(std::move(expression)) {}

  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  void ExpressionSubscriptions<InputType, OutputType,
      ServiceProtocolClientType>::Initialize(ServiceProtocolClient& client,
      int id, const Range& range, std::unique_ptr<Evaluator> filter,
      ExpressionQuery::UpdatePolicy updatePolicy,
      std::unique_ptr<Evaluator> expression) {
    auto& subscriptionEntries = m_initializingSubscriptions.Get(&client);
    auto subscriptionEntry = std::make_shared<SyncSubscriptionEntry>(id, client,
      range, std::move(filter), updatePolicy, std::move(expression));
    bool isIdUnique = subscriptionEntries.Insert(id, subscriptionEntry);
    if(!isIdUnique) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Query already exists."));
    }
    m_subscriptions.With(
      [&] (std::vector<std::shared_ptr<SyncSubscriptionEntry>>&
          subscriptionList) {
        auto insertIterator = std::lower_bound(subscriptionList.begin(),
          subscriptionList.end(), subscriptionEntry,
          [] (const std::shared_ptr<SyncSubscriptionEntry>& lhs,
              const std::shared_ptr<SyncSubscriptionEntry>& rhs) {
            auto lhsClient = Threading::With(*lhs,
              [] (const SubscriptionEntry& entry) {
                return entry.m_client;
              });
            auto rhsClient = Threading::With(*rhs,
              [] (const SubscriptionEntry& entry) {
                return entry.m_client;
              });
            return lhsClient < rhsClient;
          });
        subscriptionList.insert(insertIterator, subscriptionEntry);
      });
  }

  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  template<typename F>
  void ExpressionSubscriptions<InputType, OutputType,
      ServiceProtocolClientType>::Commit(const ServiceProtocolClient& client,
      const SnapshotLimit& snapshotLimit,
      QueryResult<SequencedValue<Output>> result,
      std::vector<SequencedValue<Input>> snapshot, const F& f) {
    auto subscriptionEntries = m_initializingSubscriptions.Find(&client);
    if(!subscriptionEntries.is_initialized()) {
      return;
    }
    auto subscriptionEntry = subscriptionEntries->FindValue(result.m_queryId);
    if(!subscriptionEntry.is_initialized()) {
      return;
    }
    subscriptionEntries->Erase(result.m_queryId);
    std::vector<SequencedValue<Output>> headBuffer;
    boost::circular_buffer<SequencedValue<Output>> tailBuffer;
    if(snapshotLimit.GetType() == SnapshotLimit::Type::TAIL) {
      tailBuffer.resize(snapshotLimit.GetSize());
    }
    Threading::With(**subscriptionEntry,
      [&] (SubscriptionEntry& subscriptionEntry) {
        if(snapshot.empty()) {
          snapshot = std::move(subscriptionEntry.m_writeLog);
          std::vector<SequencedValue<Input>>().swap(
            subscriptionEntry.m_writeLog);
        } else {
          auto mergeIterator = std::find_if(
            subscriptionEntry.m_writeLog.begin(),
            subscriptionEntry.m_writeLog.end(),
            [&] (const SequencedValue<Input>& value) {
              return value.GetSequence() > snapshot.back().GetSequence();
            });
          snapshot.insert(snapshot.end(), mergeIterator,
            subscriptionEntry.m_writeLog.end());
          std::vector<SequencedValue<Input>>().swap(
            subscriptionEntry.m_writeLog);
        }
        for(auto& data : snapshot) {
          try {
            auto value = subscriptionEntry.m_expression->template Eval<Output>(
              *data);
            if(subscriptionEntry.m_updatePolicy ==
                ExpressionQuery::UpdatePolicy::CHANGE) {
              if(subscriptionEntry.m_previousValue.is_initialized() &&
                  *subscriptionEntry.m_previousValue == value) {
                continue;
              }
              subscriptionEntry.m_previousValue = value;
            }
            if(snapshotLimit.GetType() == SnapshotLimit::Type::TAIL) {
              tailBuffer.push_back(SequencedValue(value, data.GetSequence()));
            } else {
              headBuffer.push_back(SequencedValue(value, data.GetSequence()));
            }
          } catch(const std::exception&) {}
        }
        if(snapshotLimit.GetType() == SnapshotLimit::Type::TAIL) {
          result.m_snapshot.insert(result.m_snapshot.begin(),
            tailBuffer.begin(), tailBuffer.end());
        } else {
          result.m_snapshot = std::move(headBuffer);
        }
        subscriptionEntry.m_state = SubscriptionEntry::State::COMMITTED;
        f(std::move(result));
      });
  }

  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  void ExpressionSubscriptions<InputType, OutputType,
      ServiceProtocolClientType>::End(const ServiceProtocolClient& client,
      int id) {
    m_subscriptions.RemoveIf(
      [&] (const std::shared_ptr<SyncSubscriptionEntry>& entry) {
        return Threading::With(*entry,
          [&] (const SubscriptionEntry& entry) {
            return entry.m_client == &client && entry.m_id == id;
          });
      });
  }

  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  void ExpressionSubscriptions<InputType, OutputType,
      ServiceProtocolClientType>::RemoveAll(ServiceProtocolClient& client) {
    m_subscriptions.RemoveIf(
      [&] (const std::shared_ptr<SyncSubscriptionEntry>& entry) {
        return Threading::With(*entry,
          [&] (const SubscriptionEntry& entry) {
            return entry.m_client == &client;
          });
      });
  }

  template<typename InputType, typename OutputType,
    typename ServiceProtocolClientType>
  template<typename Sender>
  void ExpressionSubscriptions<InputType, OutputType,
      ServiceProtocolClientType>::Publish(const SequencedValue<Input>& value,
      const Sender& sender) {
    m_subscriptions.ForEach(
      [&] (const std::shared_ptr<SyncSubscriptionEntry>& subscriptionEntry) {
        Threading::With(*subscriptionEntry,
          [&] (SubscriptionEntry& subscriptionEntry) {
            if((subscriptionEntry.m_range.GetStart() == Sequence::Present() ||
                RangePointGreaterOrEqual(value,
                subscriptionEntry.m_range.GetStart())) &&
                RangePointLesserOrEqual(value,
                subscriptionEntry.m_range.GetEnd()) &&
                TestFilter(*subscriptionEntry.m_filter, *value)) {
              if(subscriptionEntry.m_state ==
                  SubscriptionEntry::State::INITIALIZING) {
                subscriptionEntry.m_writeLog.push_back(value);
              } else {
                SequencedValue<Output> output;
                output.GetSequence() = value.GetSequence();
                try {
                  output.GetValue() =
                    subscriptionEntry.m_expression->template Eval<Output>(
                    *value);
                } catch(const std::exception&) {
                  return;
                }
                if(subscriptionEntry.m_updatePolicy ==
                    ExpressionQuery::UpdatePolicy::CHANGE) {
                  if(subscriptionEntry.m_previousValue.is_initialized() &&
                      *subscriptionEntry.m_previousValue == output.GetValue()) {
                    return;
                  }
                  subscriptionEntry.m_previousValue = output.GetValue();
                }
                sender(*subscriptionEntry.m_client, subscriptionEntry.m_id,
                  output);
              }
            }
        });
      });
  }
}
}

#endif
