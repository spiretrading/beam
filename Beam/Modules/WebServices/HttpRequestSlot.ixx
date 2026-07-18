module;
#include "Prelude.hpp"

export module Beam:HttpRequestSlot;

import :HttpRequest;
import :HttpResponse;

export namespace Beam {

  /** Composes an HttpRequestPredicate with a callback. */
  struct HttpRequestSlot {

    /**
     * Defines the function used to match an HTTP request.
     * @param request The HttpRequest to test.
     * @return <code>true</code> iff the request matches the predicate.
     */
    using Predicate = std::function<bool (const HttpRequest& request)>;

    /**
     * Defines the callback invoked if a predicate matches.
     * @param request The request that satisfied the predicate.
     * @return The response for the specified request.
     */
    using Slot = std::function<HttpResponse (const HttpRequest& request)>;

    /** The predicate to match. */
    Predicate m_predicate;

    /** The slot to call if the predicate matches. */
    Slot m_slot;
  };
}

