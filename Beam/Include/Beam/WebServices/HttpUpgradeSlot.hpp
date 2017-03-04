#ifndef BEAM_HTTPUPGRADESLOT_HPP
#define BEAM_HTTPUPGRADESLOT_HPP
#include <functional>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \struct HttpUpgradeSlot
      \brief Composes an HttpRequestPredicate with a callback.
      \tparam ChannelType The Channel that submitted the request.
   */
  template<typename ChannelType>
  struct HttpUpgradeSlot {

    //! The Channel that submitted the request.
    using Channel = GetTryDereferenceType<ChannelType>;

    //! Defines the function used to match an HTTP request.
    /*!
      \param request The HttpRequest to test.
      \return <code>true</code> iff the <i>request</i> matches the predicate.
    */
    using Predicate = std::function<bool (const HttpRequest& request)>;

    //! Defines the callback invoked if a predicate matches.
    /*!
      \param request The request that satisfied the predicate.
      \param channel The Channel that submitted the request.
    */
    using Slot = std::function<void (const HttpRequest& request,
      std::unique_ptr<ChannelType> channel)>;

    //! The predicate to match.
    Predicate m_predicate;

    //! The slot to call if the predicate matches.
    Slot m_slot;

    //! Constructs an HttpRequestSlot.
    /*!
      \param predicate The predicate that must be satisfied.
      \param slot The slot to call if the <i>predicate</i> is satisfied.
    */
    HttpUpgradeSlot(Predicate predicate, Slot slot);
  };

  template<typename ChannelType>
  HttpUpgradeSlot<ChannelType>::HttpUpgradeSlot(Predicate predicate, Slot slot)
      : m_predicate{std::move(predicate)},
        m_slot{std::move(slot)} {}
}
}

#endif
