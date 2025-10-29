#ifndef BEAM_EXPRESSION_SUBSCRIPTIONS_HPP
#define BEAM_EXPRESSION_SUBSCRIPTIONS_HPP
#include <memory>
#include <vector>
#include <boost/atomic/atomic.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/ExpressionQuery.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {

  /**
   * Keeps track of streaming subscriptions to expression based queries.
   * @tparam I The type of data being input to the expression.
   * @tparam O The type of data being output by the expression.
   * @tparam C The type of ServiceProtocolClients subscribing to queries.
   */
  template<typename I, typename O, typename C>
  class ExpressionSubscriptions {
    public:

      /** The type of data being input to the expression. */
      using Input = I;

      /** The type of data being output by the expression. */
      using Output = O;

      /** The type of ServiceProtocolClients subscribing to queries. */
      using ServiceProtocolClient = C;

      /** Constructs an ExpressionSubscriptions object. */
      ExpressionSubscriptions() = default;

      /**
       * Initializes an expression based subscription.
       * @param client The client initializing the subscription.
       * @param id The id used by the client to identify this query.
       * @param range The Range of the query.
       * @param filter The filter to apply to published values.
       * @param update_policy Specifies when updates should be published.
       * @param expression The expression to apply to the query.
       */
      void init(ServiceProtocolClient& client, int id, const Range& range,
        std::unique_ptr<Evaluator> filter,
        ExpressionQuery::UpdatePolicy update_policy,
        std::unique_ptr<Evaluator> expression);

      /**
       * Commits a previously initialized subscription.
       * @param client The client committing the subscription.
       * @param snapshot_limit The limits used when calculating the snapshot.
       * @param result The result of the query.
       * @param snapshot The snapshot used.
       * @param f The function to call with the result of the query.
       */
      template<typename F>
      void commit(const ServiceProtocolClient& client,
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
      void publish(const SequencedValue<Input>& value, const Sender& sender);

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
        ExpressionQuery::UpdatePolicy m_update_policy;
        std::unique_ptr<Evaluator> m_expression;
        boost::optional<Output> m_previous_value;
        std::vector<SequencedValue<Input>> m_write_log;

        SubscriptionEntry(int id, ServiceProtocolClient& client,
          const Range& range, std::unique_ptr<Evaluator> filter,
          ExpressionQuery::UpdatePolicy update_policy,
          std::unique_ptr<Evaluator> expression);
      };
      using SyncSubscriptionEntry = Sync<SubscriptionEntry>;
      SynchronizedVector<std::shared_ptr<SyncSubscriptionEntry>>
        m_subscriptions;
      SynchronizedUnorderedMap<const ServiceProtocolClient*,
        SynchronizedUnorderedMap<int, std::shared_ptr<SyncSubscriptionEntry>>>
          m_initializing_subscriptions;

      ExpressionSubscriptions(const ExpressionSubscriptions&) = delete;
      ExpressionSubscriptions& operator =(
        const ExpressionSubscriptions&) = delete;
  };

  template<typename I, typename O, typename C>
  ExpressionSubscriptions<I, O, C>::SubscriptionEntry::SubscriptionEntry(
    int id, ServiceProtocolClient& client, const Range& range,
    std::unique_ptr<Evaluator> filter,
    ExpressionQuery::UpdatePolicy update_policy,
    std::unique_ptr<Evaluator> expression)
    : m_state(State::INITIALIZING),
      m_id(id),
      m_client(&client),
      m_range(range),
      m_filter(std::move(filter)),
      m_update_policy(update_policy),
      m_expression(std::move(expression)) {}

  template<typename I, typename O, typename C>
  void ExpressionSubscriptions<I, O, C>::init(
      ServiceProtocolClient& client, int id, const Range& range,
      std::unique_ptr<Evaluator> filter,
      ExpressionQuery::UpdatePolicy update_policy,
      std::unique_ptr<Evaluator> expression) {
    auto& entries = m_initializing_subscriptions.get(&client);
    auto entry = std::make_shared<SyncSubscriptionEntry>(id, client, range,
      std::move(filter), update_policy, std::move(expression));
    if(!entries.insert(id, entry)) {
      boost::throw_with_location(std::runtime_error("Query already exists."));
    }
    m_subscriptions.with([&] (auto& subscriptions) {
      auto i =
        std::lower_bound(subscriptions.begin(), subscriptions.end(), entry,
        [] (const auto& lhs, const auto& rhs) {
          auto lhs_client = with(*lhs, [] (const auto& entry) {
            return entry.m_client;
          });
          auto rhs_client = with(*rhs, [] (const auto& entry) {
            return entry.m_client;
          });
          return lhs_client < rhs_client;
        });
      subscriptions.insert(i, entry);
    });
  }

  template<typename I, typename O, typename C>
  template<typename F>
  void ExpressionSubscriptions<I, O, C>::commit(
      const ServiceProtocolClient& client, const SnapshotLimit& snapshot_limit,
      QueryResult<SequencedValue<Output>> result,
      std::vector<SequencedValue<Input>> snapshot, F&& f) {
    auto entries = m_initializing_subscriptions.find(&client);
    if(!entries) {
      return;
    }
    auto entry = entries->try_load(result.m_id);
    if(!entry) {
      return;
    }
    entries->erase(result.m_id);
    auto head_buffer = std::vector<SequencedValue<Output>>();
    auto tail_buffer =
      boost::circular_buffer_space_optimized<SequencedValue<Output>>(
        snapshot_limit.get_size());
    with(**entry, [&] (auto& entry) {
      if(snapshot.empty()) {
        snapshot = std::move(entry.m_write_log);
      } else {
        auto i =
          std::find_if(entry.m_write_log.begin(), entry.m_write_log.end(),
            [&] (const auto& value) {
              return value.get_sequence() > snapshot.back().get_sequence();
            });
        snapshot.insert(snapshot.end(), i, entry.m_write_log.end());
      }
      entry.m_write_log = {};
      for(auto& data : snapshot) {
        try {
          auto value = entry.m_expression->template eval<Output>(*data);
          if(entry.m_update_policy == ExpressionQuery::UpdatePolicy::CHANGE) {
            if(entry.m_previous_value && *entry.m_previous_value == value) {
              continue;
            }
            entry.m_previous_value = value;
          }
          if(snapshot_limit.get_type() == SnapshotLimit::Type::TAIL) {
            tail_buffer.push_back(SequencedValue(value, data.get_sequence()));
          } else {
            head_buffer.push_back(SequencedValue(value, data.get_sequence()));
          }
        } catch(const std::exception&) {}
      }
      if(snapshot_limit.get_type() == SnapshotLimit::Type::TAIL) {
        result.m_snapshot.insert(
          result.m_snapshot.begin(), tail_buffer.begin(), tail_buffer.end());
      } else {
        result.m_snapshot = std::move(head_buffer);
      }
      entry.m_state = SubscriptionEntry::State::COMMITTED;
      std::forward<F>(f)(std::move(result));
    });
  }

  template<typename I, typename O, typename C>
  void ExpressionSubscriptions<I, O, C>::end(
      const ServiceProtocolClient& client, int id) {
    m_subscriptions.erase_if([&] (const auto& entry) {
      return with(*entry, [&] (const auto& entry) {
        return entry.m_client == &client && entry.m_id == id;
      });
    });
  }

  template<typename I, typename O, typename C>
  void ExpressionSubscriptions<I, O, C>::remove_all(
      ServiceProtocolClient& client) {
    m_subscriptions.erase_if([&] (const auto& entry) {
      return with(*entry, [&] (const auto& entry) {
        return entry.m_client == &client;
      });
    });
  }

  template<typename I, typename O, typename C>
  template<typename Sender>
  void ExpressionSubscriptions<I, O, C>::publish(
      const SequencedValue<Input>& value, const Sender& sender) {
    m_subscriptions.for_each([&] (const auto& entry) {
      with(*entry, [&] (auto& entry) {
        if((entry.m_range.get_start() == Sequence::PRESENT ||
            range_point_greater_or_equal(value, entry.m_range.get_start())) &&
              range_point_lesser_or_equal(value, entry.m_range.get_end()) &&
                test_filter(*entry.m_filter, *value)) {
          if(entry.m_state == SubscriptionEntry::State::INITIALIZING) {
            entry.m_write_log.push_back(value);
          } else {
            auto output = SequencedValue<Output>();
            output.get_sequence() = value.get_sequence();
            try {
              output.get_value() =
                entry.m_expression->template eval<Output>(*value);
            } catch(const std::exception&) {
              return;
            }
            if(entry.m_update_policy == ExpressionQuery::UpdatePolicy::CHANGE) {
              if(entry.m_previous_value == output.get_value()) {
                return;
              }
              entry.m_previous_value = output.get_value();
            }
            sender(*entry.m_client, entry.m_id, output);
          }
        }
      });
    });
  }
}

#endif
