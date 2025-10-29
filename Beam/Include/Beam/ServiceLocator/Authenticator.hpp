#ifndef BEAM_AUTHENTICATOR_HPP
#define BEAM_AUTHENTICATOR_HPP
#include <concepts>
#include "Beam/Services/ServiceProtocolClient.hpp"

namespace Beam {
namespace Details {
  template<typename T>
  struct is_service_protocol_client : std::false_type {};

  template<typename M, typename T, typename P, typename S, bool V>
  struct is_service_protocol_client<ServiceProtocolClient<M, T, P, S, V>> :
    std::true_type {};

  template<typename T>
  struct is_service_protocol_client<const T> : is_service_protocol_client<T> {};

  template<typename T>
  struct is_service_protocol_client<volatile T> :
    is_service_protocol_client<T> {};

  template<typename T>
  struct is_service_protocol_client<const volatile T> :
    is_service_protocol_client<T> {};

  template<typename T>
  inline constexpr auto is_service_protocol_client_v =
    is_service_protocol_client<T>::value;
}

  /**
   * Specifies an invocable that authenticates a ServiceProtocolClient.
   * @tparam F The invocable type.
   * @tparam C The type of ServiceProtocolClient to authenticate.
   */
  template<typename F, typename C>
  concept Authenticator = Details::is_service_protocol_client_v<C> &&
    std::invocable<F, C&> && std::same_as<std::invoke_result_t<F, C&>, void>;

  /**
   * Authenticates a ServiceProtocolClient.
   * @param authenticator The Authenticator to use.
   * @param client The ServiceProtocolClient to authenticate.
   */
  template<typename M, typename T, typename P, typename S, bool V,
    Authenticator<ServiceProtocolClient<M, T, P, S, V>> F>
  void authenticate(
      const F& authenticator, ServiceProtocolClient<M, T, P, S, V>& client) {
    authenticator(client);
  }
}

#endif
