#ifndef AVALON_HTTPSESSIONREQUESTSLOT_HPP
#define AVALON_HTTPSESSIONREQUESTSLOT_HPP
#include "Avalon/WebServices/WebServices.hpp"

namespace Avalon {
namespace WebServices {

  /*! \struct HttpSessionRequestSlot
      \brief Stores a request slot for an HttpSession.
   */
  struct HttpSessionRequestSlot {

    //! Defines the callback invoked if a predicate matches.
    /*!
      \param session The HttpSession that made the request.
      \param request The request that satisfied the predicate.
      \param response The response for the specified <i>request</i>.
    */
    typedef boost::function<void (HttpSession*, HttpServerRequest*,
      HttpServerResponse*)> Slot;

    //! The predicate to match.
    HttpRequestPredicate m_predicate;

    //! The slot to call if the predicate matches.
    Slot m_slot;

    //! Constructs an HttpRequestSlot.
    /*!
      \param predicate The predicate that must be satisfied.
      \param slot The slot to call if the <i>predicate</i> is satisfied.
    */
    HttpSessionRequestSlot(const HttpRequestPredicate& predicate,
      const Slot& slot);
  };
}
}

#endif // AVALON_HTTPSESSIONREQUESTSLOT_HPP
