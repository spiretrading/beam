#ifndef BEAM_PUBLISHER_REACTOR_HPP
#define BEAM_PUBLISHER_REACTOR_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename PublisherType>
  struct PublisherReactorCore {
    using Publisher = PublisherType;
    using Type = typename GetTryDereferenceType<Publisher>::Type;
    Publisher m_publisher;

    template<typename PublisherForward>
    PublisherReactorCore(PublisherForward&& publisher)
        : m_publisher{std::forward<PublisherForward>(publisher)} {}

    Type operator ()(const Type& type) const {
      return type;
    }
  };
}

  //! Makes a Reactor that monitors a Publisher.
  /*!
    \param publisher The Publisher to monitor.
  */
  template<typename Type>
  auto MakePublisherReactor(const Publisher<Type>& publisher) {
    auto queue = std::make_shared<Queue<Type>>();
    publisher.Monitor(queue);
    return MakeQueueReactor(queue);
  }

  //! Makes a Reactor that monitors a Publisher, used when ownership of the
  //! Publisher is managed by the Reactor.
  /*!
    \param publisher The Publisher to monitor.
  */
  template<typename Publisher>
  auto MakePublisherReactor(std::shared_ptr<Publisher> publisher) {
    auto baseReactor = MakePublisherReactor(*publisher);
    return MakeFunctionReactor(Details::PublisherReactorCore<
      std::shared_ptr<Publisher>>{std::move(publisher)}, baseReactor);
  }
}
}

#endif
