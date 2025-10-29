#ifndef BEAM_WRAPPER_CHANNEL_HPP
#define BEAM_WRAPPER_CHANNEL_HPP
#include <type_traits>
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {
namespace Details {
  template<typename T, int>
  struct ComponentHolder {
    local_ptr_t<T> m_component;

    ComponentHolder() = default;

    template<Initializes<T> TF>
    ComponentHolder(TF&& component)
      : m_component(std::forward<TF>(component)) {}

    T& get_component() {
      return *m_component;
    }

    const T& get_component() const {
      return *m_component;
    }
  };

  template<int I>
  struct ComponentHolder<void, I> {
    ComponentHolder() = default;
  };
}

  /**
   * Implements a Channel by composing an existing Channel with up to three
   * optional component types.
   * @tparam C The type of Channel to wrap.
   * @tparam CT1 The type of the first optional component to compose.
   * @tparam CT2 The type of the second optional component to compose.
   * @tparam CT3 The type of the third optional component to compose.
   */
  template<typename C, typename CT1 = void, typename CT2 = void,
    typename CT3 = void>
  class WrapperChannel :
      private Details::ComponentHolder<C, 0>,
      private Details::ComponentHolder<CT1, 1>,
      private Details::ComponentHolder<CT2, 2>,
      private Details::ComponentHolder<CT3, 3> {
    private:
      using Component1 = dereference_t<CT1>;
      using Component2 = dereference_t<CT2>;
      using Component3 = dereference_t<CT3>;

    public:

      /** The type of Channel to wrap. */
      using Channel = dereference_t<C>;

      using Identifier = typename Channel::Identifier;
      using Connection =
        std::conditional_t<IsConnection<Component1>, Component1,
          std::conditional_t<IsConnection<Component2>, Component2,
            std::conditional_t<IsConnection<Component3>, Component3,
              typename Channel::Connection>>>;
      using Reader = std::conditional_t<IsReader<Component1>, Component1,
        std::conditional_t<IsReader<Component2>, Component2,
          std::conditional_t<IsReader<Component3>, Component3,
            typename Channel::Reader>>>;
      using Writer = std::conditional_t<IsWriter<Component1>, Component1,
        std::conditional_t<IsWriter<Component2>, Component2,
          std::conditional_t<IsWriter<Component3>, Component3,
            typename Channel::Writer>>>;

      /**
       * Constructs a WrapperChannel that owns or references a channel.
       * @param channel The channel to wrap.
       */
      template<Initializes<C> CF>
      explicit WrapperChannel(CF&& channel);

      /**
       * Constructs a WrapperChannel with one overriding component.
       * @param channel The channel to wrap.
       * @param component1 The first component that may supply/override roles.
       */
      template<Initializes<C> CF, Initializes<CT1> CF1>
      WrapperChannel(CF&& channel, CF1&& component1);

      /**
       * Constructs a WrapperChannel with two overriding components.
       * @param channel The channel to wrap.
       * @param component1 The first component that may supply/override roles.
       * @param component2 The second component that may supply/override roles.
       */
      template<Initializes<C> CF, Initializes<CT1> CF1, Initializes<CT2> CF2>
      WrapperChannel(CF&& channel, CF1&& component1, CF2&& component2);

      /**
       * Constructs a WrapperChannel with three overriding components.
       * @param channel The channel to wrap.
       * @param component1 The first component that may supply/override roles.
       * @param component2 The second component that may supply/override roles.
       * @param component3 The third component that may supply/override roles.
       */
      template<Initializes<C> CF, Initializes<CT1> CF1, Initializes<CT2> CF2,
        Initializes<CT3> CF3>
      WrapperChannel(
        CF&& channel, CF1&& component1, CF2&& component2, CF3&& component3);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();
  };

  template<typename CF>
  WrapperChannel(CF&&) -> WrapperChannel<std::remove_cvref_t<CF>>;

  template<typename CF, typename C1>
  WrapperChannel(CF&&, C1&&) ->
    WrapperChannel<std::remove_cvref_t<CF>, std::remove_cvref_t<C1>>;

  template<typename CF, typename C1, typename C2>
  WrapperChannel(CF&&, C1&&, C2&&) ->
    WrapperChannel<std::remove_cvref_t<CF>, std::remove_cvref_t<C1>,
      std::remove_cvref_t<C2>>;

  template<typename CF, typename C1, typename C2, typename C3>
  WrapperChannel(CF&&, C1&&, C2&&, C3&&) ->
    WrapperChannel<std::remove_cvref_t<CF>, std::remove_cvref_t<C1>,
      std::remove_cvref_t<C2>, std::remove_cvref_t<C3>>;

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<Initializes<C> CF>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(CF&& channel)
    : Details::ComponentHolder<C, 0>(std::forward<CF>(channel)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<Initializes<C> CF, Initializes<CT1> CF1>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(
    CF&& channel, CF1&& component1)
    : Details::ComponentHolder<C, 0>(std::forward<CF>(channel)),
      Details::ComponentHolder<CT1, 1>(std::forward<CF1>(component1)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<Initializes<C> CF, Initializes<CT1> CF1, Initializes<CT2> CF2>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(
    CF&& channel, CF1&& component1, CF2&& component2)
    : Details::ComponentHolder<C, 0>(std::forward<CF>(channel)),
      Details::ComponentHolder<CT1, 1>(std::forward<CF1>(component1)),
      Details::ComponentHolder<CT2, 2>(std::forward<CF2>(component2)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<Initializes<C> CF, Initializes<CT1> CF1, Initializes<CT2> CF2,
    Initializes<CT3> CF3>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(
    CF&& channel, CF1&& component1, CF2&& component2, CF3&& component3)
    : Details::ComponentHolder<C, 0>(std::forward<CF>(channel)),
      Details::ComponentHolder<CT1, 1>(std::forward<CF1>(component1)),
      Details::ComponentHolder<CT2, 2>(std::forward<CF2>(component2)),
      Details::ComponentHolder<CT3, 3>(std::forward<CF3>(component3)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  const typename WrapperChannel<C, CT1, CT2, CT3>::Identifier&
      WrapperChannel<C, CT1, CT2, CT3>::get_identifier() const {
    return Details::ComponentHolder<C, 0>::get_component().get_identifier();
  }

  template<typename C, typename CT1, typename CT2, typename CT3>
  typename WrapperChannel<C, CT1, CT2, CT3>::Connection&
      WrapperChannel<C, CT1, CT2, CT3>::get_connection() {
    if constexpr(IsConnection<Component1>) {
      return Details::ComponentHolder<CT1, 1>::get_component();
    } else if constexpr(IsConnection<Component2>) {
      return Details::ComponentHolder<CT2, 2>::get_component();
    } else if constexpr(IsConnection<Component3>) {
      return Details::ComponentHolder<CT3, 3>::get_component();
    } else {
      return Details::ComponentHolder<C, 0>::get_component().get_connection();
    }
  }

  template<typename C, typename CT1, typename CT2, typename CT3>
  typename WrapperChannel<C, CT1, CT2, CT3>::Reader&
      WrapperChannel<C, CT1, CT2, CT3>::get_reader() {
    if constexpr(IsReader<Component1>) {
      return Details::ComponentHolder<CT1, 1>::get_component();
    } else if constexpr(IsReader<Component2>) {
      return Details::ComponentHolder<CT2, 2>::get_component();
    } else if constexpr(IsReader<Component3>) {
      return Details::ComponentHolder<CT3, 3>::get_component();
    } else {
      return Details::ComponentHolder<C, 0>::get_component().get_reader();
    }
  }

  template<typename C, typename CT1, typename CT2, typename CT3>
  typename WrapperChannel<C, CT1, CT2, CT3>::Writer&
      WrapperChannel<C, CT1, CT2, CT3>::get_writer() {
    if constexpr(IsWriter<Component1>) {
      return Details::ComponentHolder<CT1, 1>::get_component();
    } else if constexpr(IsWriter<Component2>) {
      return Details::ComponentHolder<CT2, 2>::get_component();
    } else if constexpr(IsWriter<Component3>) {
      return Details::ComponentHolder<CT3, 3>::get_component();
    } else {
      return Details::ComponentHolder<C, 0>::get_component().get_writer();
    }
  }
}

#endif
