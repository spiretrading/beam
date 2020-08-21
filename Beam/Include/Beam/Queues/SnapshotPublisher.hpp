#ifndef BEAM_SNAPSHOT_PUBLISHER_HPP
#define BEAM_SNAPSHOT_PUBLISHER_HPP
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {
  class BaseSnapshotPublisher {
    public:
      virtual ~BaseSnapshotPublisher() = default;
  };

  /**
   * Interface for a Publisher that has a snapshot.
   * @param <T> The type of data being published.
   * @param <S> The type of snapshot stored.
   */
  template<typename T, typename S>
  class SnapshotPublisher : public Publisher<T>, public BaseSnapshotPublisher {
    public:

      /** The type of data published. */
      using Type = typename Publisher<T>::Type;

      /** The type of snapshot stored. */
      using Snapshot = S;

      /** Constructs a SnapshotPublisher. */
      SnapshotPublisher() = default;

      /**
       * Performs a synchronized action with this Publisher's Snapshot.
       * @param f The action to perform.
       */
      template<typename F, typename = std::enable_if_t<
        !std::is_same_v<std::invoke_result_t<F,
        boost::optional<const Snapshot&>>, void>>>
      decltype(auto) With(F&& f) const;

      /**
       * Performs a synchronized action with this Publisher's Snapshot.
       * @param f The action to perform.
       */
      virtual void With(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const = 0;

      /** Returns the snapshot. */
      virtual boost::optional<Snapshot> GetSnapshot() const;

      /**
       * Monitors updates to this SnapshotPublisher.
       * @param monitor The monitor to publish updates to.
       * @param snapshot Where to store the current Snapshot.
       */
      virtual void Monitor(ScopedQueueWriter<Type> monitor,
        Out<boost::optional<Snapshot>> snapshot) const = 0;

      using Publisher<T>::With;
      using Publisher<T>::Monitor;
  };

  template<typename T, typename S>
  template<typename F, typename>
  decltype(auto) SnapshotPublisher<T, S>::With(F&& f) const {
    using R = std::invoke_result_t<F, boost::optional<const Snapshot&>>;
    if constexpr(std::is_reference_v<R>) {
      auto result = static_cast<std::remove_reference_t<R>*>(nullptr);
      With([&] (auto snapshot) {
        result = &f(std::move(snapshot));
      });
      return *result;
    } else {
      auto result = boost::optional<R>();
      With([&] (auto snapshot) {
        result.emplace(f(std::move(snapshot)));
      });
      return R(std::move(*result));
    }
  }

  template<typename T, typename S>
  boost::optional<typename SnapshotPublisher<T, S>::Snapshot>
      SnapshotPublisher<T, S>::GetSnapshot() const {
    return With([] (auto s) -> boost::optional<Snapshot> {
      if(s) {
        return std::move(*s);
      }
      return boost::none;
    });
  }

namespace Details {
  template<typename PublisherType, bool IsSnapshot = false>
  struct GetPublisherTypeHelper {
    using type = Publisher<typename PublisherType::Type>;
  };

  template<typename PublisherType>
  struct GetPublisherTypeHelper<PublisherType, true> {
    using type = SnapshotPublisher<typename PublisherType::Type,
      typename PublisherType::Snapshot>;
  };

  template<typename PublisherType>
  struct GetPublisherType {
    using type = typename GetPublisherTypeHelper<PublisherType,
      std::is_base_of_v<BaseSnapshotPublisher, PublisherType>>::type;
  };

  template<typename PublisherType, bool IsSnapshot = false>
  struct GetSnapshotTypeHelper {
    using type = int;
  };

  template<typename PublisherType>
  struct GetSnapshotTypeHelper<PublisherType, true> {
    using type = typename PublisherType::Snapshot;
  };

  template<typename PublisherType>
  struct GetSnapshotType {
    using type = typename GetSnapshotTypeHelper<PublisherType,
      std::is_base_of_v<BaseSnapshotPublisher, PublisherType>>::type;
  };
}
}

#endif
