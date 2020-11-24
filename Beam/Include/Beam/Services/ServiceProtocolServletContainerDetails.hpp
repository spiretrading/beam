#ifndef BEAM_SERVICE_PROTOCOL_SERVLET_CONTAINER_DETAILS_HPP
#define BEAM_SERVICE_PROTOCOL_SERVLET_CONTAINER_DETAILS_HPP
#include <type_traits>

namespace Beam::Services::Details {
  template<bool HasMethod>
  struct InvokeClientAccepted {
    template<typename ServletType, typename ClientType>
    void operator ()(ServletType& servlet, ClientType& client) const {
      servlet.HandleClientAccepted(client);
    }
  };

  template<>
  struct InvokeClientAccepted<false> {
    template<typename ServletType, typename ClientType>
    void operator ()(ServletType& servlet, ClientType& client) const {}
  };

  template<typename ServletType, typename ClientType>
  struct HasClientAcceptedMethod {
    using YesType = char;
    using NoType = struct {
      char a[2];
    };

    template<typename C>
    static YesType Test(decltype(std::declval<C>().HandleClientAccepted(
      std::declval<ClientType&>()))*);

    template<typename C>
    static NoType Test(...);

    static constexpr auto value = sizeof(Test<ServletType>(nullptr)) ==
      sizeof(YesType);
  };


  template<bool HasMethod>
  struct InvokeClientClosed {
    template<typename ServletType, typename ClientType>
    void operator ()(ServletType& servlet, ClientType& client) const {
      servlet.HandleClientClosed(client);
    }
  };

  template<>
  struct InvokeClientClosed<false> {
    template<typename ServletType, typename ClientType>
    void operator ()(ServletType& servlet, ClientType& client) const {}
  };

  template<typename ServletType, typename ClientType>
  struct HasClientClosedMethod {
    using YesType = char;
    using NoType = struct { char a[2]; };

    template<typename C>
    static YesType Test(decltype(std::declval<C>().HandleClientClosed(
      std::declval<ClientType&>()))*);

    template<typename C>
    static NoType Test(...);

    static const bool value = sizeof(Test<ServletType>(nullptr)) ==
      sizeof(YesType);
  };
}

#endif
