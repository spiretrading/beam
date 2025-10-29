#ifndef BEAM_HTTP_UPGRADE_SLOT_HPP
#define BEAM_HTTP_UPGRADE_SLOT_HPP
#include <functional>
#include <memory>
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /**
   * Composes an HttpRequestPredicate with a callback.
   * @tparam C The Channel that submitted the request.
   */
  template<typename C> requires IsChannel<dereference_t<C>>
  struct HttpUpgradeSlot {

    /** The Channel that submitted the request. */
    using Channel = dereference_t<C>;

    /**
     * Defines the function used to match an HTTP request.
     * @param request The HttpRequest to test.
     * @return <code>true</code> iff the request matches the predicate.
     */
    using Predicate = std::function<bool (const HttpRequest& request)>;

    /**
     * Defines the callback invoked if a predicate matches.
     * @param request The request that satisfied the predicate.
     * @param channel The Channel that submitted the request.
     */
    using Slot = std::function<
      void (const HttpRequest& request, std::unique_ptr<C> channel)>;

    /** The predicate to match. */
    Predicate m_predicate;

    /** The slot to call if the predicate matches. */
    Slot m_slot;
  };
}

#endif
