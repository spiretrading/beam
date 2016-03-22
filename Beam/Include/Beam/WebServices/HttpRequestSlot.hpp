#ifndef BEAM_HTTPREQUESTSLOT_HPP
#define BEAM_HTTPREQUESTSLOT_HPP
#include <functional>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \struct HttpRequestSlot
      \brief Composes an HttpRequestPredicate with a callback.
   */
  struct HttpRequestSlot {

    //! Defines the function used to match an HTTP request.
    /*!
      \param request The HttpRequest to test.
      \return <code>true</code> iff the <i>request</i> matches the predicate.
    */
    using Predicate = std::function<bool (const HttpRequest& request)>;

    //! Defines the callback invoked if a predicate matches.
    /*!
      \param request The request that satisfied the predicate.
      \param response The response for the specified <i>request</i>.
    */
    using Slot = std::function<HttpServerResponse (const HttpRequest& request)>;

    //! The predicate to match.
    Predicate m_predicate;

    //! The slot to call if the predicate matches.
    Slot m_slot;

    //! Constructs an HttpRequestSlot.
    /*!
      \param predicate The predicate that must be satisfied.
      \param slot The slot to call if the <i>predicate</i> is satisfied.
    */
    HttpRequestSlot(Predicate predicate, Slot slot);
  };

  inline HttpRequestSlot::HttpRequestSlot(Predicate predicate, Slot slot)
      : m_predicate{std::move(predicate)},
        m_slot{std::move(slot)} {}
}
}

#endif
