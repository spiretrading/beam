#ifndef BEAM_PUBLISHER_REACTOR_HPP
#define BEAM_PUBLISHER_REACTOR_HPP
#include <memory>
#include <Aspen/Lift.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReactor.hpp"

namespace Beam {
namespace Details {
  template<typename P>
  struct PublisherReactorCore {
    using Publisher = P;
    using Type = typename dereference_t<Publisher>::Type;
    Publisher m_publisher;

    template<typename PF>
    PublisherReactorCore(PF&& publisher)
      : m_publisher(std::forward<PF>(publisher)) {}

    const Type& operator ()(const Type& type) const {
      return type;
    }
  };
}

  /**
   * Makes a Reactor that monitors a Publisher.
   * @param publisher The Publisher to monitor.
   */
  template<typename T>
  auto publisher_reactor(const Publisher<T>& publisher) {
    auto queue = std::make_shared<Queue<T>>();
    publisher.monitor(queue);
    return QueueReactor(std::move(queue));
  }

  /**
   * Makes a Reactor that monitors a Publisher, used when ownership of the
   * Publisher is managed by the Reactor.
   * @param publisher The Publisher to monitor.
   */
  template<typename P>
  auto publisher_reactor(std::shared_ptr<P> publisher) {
    return Aspen::lift(Details::PublisherReactorCore<std::shared_ptr<P>>(
      publisher), publisher_reactor(*publisher));
  }
}

#endif
