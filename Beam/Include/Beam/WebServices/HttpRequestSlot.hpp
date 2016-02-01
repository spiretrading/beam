#ifndef AVALON_HTTPREQUESTSLOT_HPP
#define AVALON_HTTPREQUESTSLOT_HPP
#include "Avalon/WebServices/WebServices.hpp"

namespace Avalon {
namespace WebServices {

  /*! \struct HttpRequestSlot
      \brief Composes an HttpRequestPredicate with a callback.
   */
  struct HttpRequestSlot {

    //! Defines the callback invoked if a predicate matches.
    /*!
      \param request The request that satisfied the predicate.
      \param response The response for the specified <i>request</i>.
    */
    typedef boost::function<void (HttpServerRequest*, HttpServerResponse*)>
      Slot;

    //! The predicate to match.
    HttpRequestPredicate m_predicate;

    //! The slot to call if the predicate matches.
    Slot m_slot;

    //! Constructs an HttpRequestSlot.
    /*!
      \param predicate The predicate that must be satisfied.
      \param slot The slot to call if the <i>predicate</i> is satisfied.
    */
    HttpRequestSlot(const HttpRequestPredicate& predicate, const Slot& slot);
  };
}
}

#endif // AVALON_HTTPREQUESTSLOT_HPP
