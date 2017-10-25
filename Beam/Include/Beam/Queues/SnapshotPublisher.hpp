#ifndef BEAM_SNAPSHOTPUBLISHER_HPP
#define BEAM_SNAPSHOTPUBLISHER_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {
  class BaseSnapshotPublisher {
    public:
      virtual ~BaseSnapshotPublisher() = default;
  };

  /*! \class SnapshotPublisher
      \brief Interface for a Publisher that has a snapshot.
      \tparam T The type of data being published.
      \tparam SnapshotType The type of snapshot stored.
   */
  template<typename T, typename SnapshotType>
  class SnapshotPublisher : public Publisher<T>, public BaseSnapshotPublisher {
    public:

      //! The type of data published.
      using Type = typename Publisher<T>::Type;

      //! The type of snapshot stored.
      using Snapshot = SnapshotType;

      //! Constructs a SnapshotPublisher.
      SnapshotPublisher() = default;

      virtual ~SnapshotPublisher() = default;

      //! Performs a synchronized action with this Publisher's Snapshot.
      /*!
        \param f The action to perform.
      */
      virtual void WithSnapshot(
        const std::function<void (boost::optional<const Snapshot&>)>& f)
        const = 0;

      //! Monitors updates to this SnapshotPublisher.
      /*!
        \param monitor The monitor to publish updates to.
        \param snapshot Where to store the current Snapshot.
      */
      virtual void Monitor(std::shared_ptr<QueueWriter<T>> monitor,
        Out<boost::optional<Snapshot>> snapshot) const = 0;

      using Publisher<T>::Monitor;
  };

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
      std::is_base_of<BaseSnapshotPublisher, PublisherType>::value>::type;
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
      std::is_base_of<BaseSnapshotPublisher, PublisherType>::value>::type;
  };
}
}

#endif
