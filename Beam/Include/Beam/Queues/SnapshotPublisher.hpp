#ifndef BEAM_SNAPSHOTPUBLISHER_HPP
#define BEAM_SNAPSHOTPUBLISHER_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {
  class BaseSnapshotPublisher {};

  /*! \class SnapshotPublisher
      \brief Interface for a Publisher that has a snapshot.
      \tparam T The type of data being published.
      \tparam SnapshotType The type of snapshot stored.
   */
  template<typename T, typename SnapshotType>
  class SnapshotPublisher : public Publisher<T>, public BaseSnapshotPublisher {
    public:

      //! The type of data published.
      typedef typename Publisher<T>::Type Type;

      //! The type of snapshot stored.
      typedef SnapshotType Snapshot;

      //! Constructs a SnapshotPublisher.
      SnapshotPublisher();

      virtual ~SnapshotPublisher();

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

  template<typename T, typename SnapshotType>
  SnapshotPublisher<T, SnapshotType>::SnapshotPublisher() {}

  template<typename T, typename SnapshotType>
  SnapshotPublisher<T, SnapshotType>::~SnapshotPublisher() {}

namespace Details {
  template<typename PublisherType, bool IsSnapshot = false>
  struct GetPublisherTypeHelper {
    typedef Publisher<typename PublisherType::Type> type;
  };

  template<typename PublisherType>
  struct GetPublisherTypeHelper<PublisherType, true> {
    typedef SnapshotPublisher<typename PublisherType::Type,
      typename PublisherType::Snapshot> type;
  };

  template<typename PublisherType>
  struct GetPublisherType {
    typedef typename GetPublisherTypeHelper<PublisherType,
      std::is_base_of<BaseSnapshotPublisher, PublisherType>::value>::type type;
  };

  template<typename PublisherType, bool IsSnapshot = false>
  struct GetSnapshotTypeHelper {
    typedef int type;
  };

  template<typename PublisherType>
  struct GetSnapshotTypeHelper<PublisherType, true> {
    typedef typename PublisherType::Snapshot type;
  };

  template<typename PublisherType>
  struct GetSnapshotType {
    typedef typename GetSnapshotTypeHelper<PublisherType,
      std::is_base_of<BaseSnapshotPublisher, PublisherType>::value>::type type;
  };
}
}

#endif
