#ifndef BEAM_PUBLISHER_HPP
#define BEAM_PUBLISHER_HPP
#include <memory>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /*! \class BasePublisher
      \brief Base class for the parametric Publisher.
   */
  class BasePublisher {
    public:
      virtual ~BasePublisher() = default;

      //! Synchronizes access to this publisher.
      /*!
        \param f The synchronized action to perform.
      */
      virtual void With(const std::function<void ()>& f) const = 0;
  };

  /*! \class Publisher
      \brief Interface for an object that publishes data to Queues.
      \tparam T The type of data being published.
   */
  template<typename T>
  class Publisher : public BasePublisher {
    public:

      //! The type of data published.
      using Type = T;

      //! Constructs a Publisher.
      Publisher() = default;

      virtual ~Publisher() override = default;

      //! Monitors updates to this Publisher.
      /*!
        \param monitor The monitor to publish updates to.
      */
      virtual void Monitor(std::shared_ptr<QueueWriter<T>> monitor) const = 0;
  };

  /*! \struct PublisherType
      \brief Specifies the type of values a Publisher produces.
      \tparam Publisher The type of Publisher to inspect.
   */
  template<typename Publisher>
  struct PublisherType {
    using type = typename GetTryDereferenceType<Publisher>::Type;
  };

  template<typename Publisher>
  using GetPublisherType = typename PublisherType<Publisher>::type;
}

#endif
