#ifndef BEAM_INITIALIZER_HPP
#define BEAM_INITIALIZER_HPP
#include <tuple>
#include <utility>

namespace Beam {

  /**
   * Wraps parameters intended to initialize a value, typically stored by a
   * managed pointer.
   */
  template<typename... Args>
  struct Initializer {

    /** Stores the parameters that are used to initialize the value. */
    std::tuple<Args&&...> m_args;

    /**
     * Constructs an Initializer.
     * @param args The parameters used to initialize the value.
     */
    template<typename... ArgForwards>
    Initializer(ArgForwards&&... args) noexcept;
  };

  /**
   * Constructs an Initializer from a list of arguments.
   * @param args The list of arguments to use as an initializer.
   */
  template<typename... Args>
  auto init(Args&&... args) noexcept {
    return Initializer<Args...>(std::forward<Args>(args)...);
  }

  template<typename T, typename... Args, std::size_t... Sequence>
  auto make(Initializer<Args...>&& initializer,
      std::index_sequence<Sequence...>) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
    return T(std::get<Sequence>(std::move(initializer.m_args))...);
  }

  template<typename T, typename... Args>
  auto make(Initializer<Args...>&& initializer) noexcept(
      std::is_nothrow_constructible_v<T, Args...>) {
    return make<T, Args...>(std::move(initializer),
      std::make_index_sequence<sizeof...(Args)>());
  }

  template<typename... Args>
  template<typename... ArgForwards>
  Initializer<Args...>::Initializer(ArgForwards&&... args) noexcept
    : m_args(std::forward<ArgForwards>(args)...) {}
}

#endif
