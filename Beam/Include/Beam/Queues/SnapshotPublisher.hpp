#ifndef BEAM_SNAPSHOT_PUBLISHER_HPP
#define BEAM_SNAPSHOT_PUBLISHER_HPP
#include <concepts>
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/Publisher.hpp"

namespace Beam {

  /**
   * Interface for a Publisher that has a snapshot.
   * @tparam T The type of data being published.
   * @tparam S The type of snapshot stored.
   */
  template<typename T, typename S>
  class SnapshotPublisher : public Publisher<T> {
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
      template<std::invocable<boost::optional<const S&>> F>
      decltype(auto) with(F&& f) const;

      /**
       * Performs a synchronized action with this Publisher's Snapshot.
       * @param f The action to perform.
       */
      virtual void with(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
          const = 0;

      /** Returns the snapshot. */
      virtual boost::optional<Snapshot> get_snapshot() const;

      /**
       * Monitors updates to this SnapshotPublisher.
       * @param monitor The monitor to publish updates to.
       * @param snapshot Where to store the current Snapshot.
       */
      virtual void monitor(ScopedQueueWriter<Type> monitor,
        Out<boost::optional<Snapshot>> snapshot) const = 0;

      using Publisher<T>::with;
      using Publisher<T>::monitor;
  };

  template<typename T, typename S>
  template<std::invocable<boost::optional<const S&>> F>
  decltype(auto) SnapshotPublisher<T, S>::with(F&& f) const {
    using R = std::invoke_result_t<F, boost::optional<const Snapshot&>>;
    using BaseWith = void (SnapshotPublisher::*)(
      const std::function<void (boost::optional<const Snapshot&>)>&) const;
    if constexpr(std::is_reference_v<R>) {
      auto result = static_cast<std::remove_cvref_t<R>*>(nullptr);
      (this->*static_cast<BaseWith>(&SnapshotPublisher::with))(
        [&] (auto snapshot) {
          result = &f(std::move(snapshot));
        });
      return *result;
    } else if constexpr(std::is_void_v<R>) {
      (this->*static_cast<BaseWith>(&SnapshotPublisher::with))(
        std::forward<F>(f));
    } else {
      auto result = boost::optional<R>();
      (this->*static_cast<BaseWith>(&SnapshotPublisher::with))(
        [&] (auto snapshot) {
          result.emplace(f(std::move(snapshot)));
        });
      return R(std::move(*result));
    }
  }

  template<typename T, typename S>
  boost::optional<typename SnapshotPublisher<T, S>::Snapshot>
      SnapshotPublisher<T, S>::get_snapshot() const {
    return with([] (auto s) -> boost::optional<Snapshot> {
      if(s) {
        return std::move(*s);
      }
      return boost::none;
    });
  }
}

#endif
