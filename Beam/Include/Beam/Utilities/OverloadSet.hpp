#ifndef BEAM_OVERLOAD_SET_HPP
#define BEAM_OVERLOAD_SET_HPP
#include <type_traits>
#include <utility>
#include <variant>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Combines multiple callables into a single overload set.
   * @tparam Ts The types of callables to combine.
   */
  template<typename... Ts>
  struct OverloadSet;

  template<typename... Ts>
  struct OverloadSet : public Ts... {
    OverloadSet(Ts... callables) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<Ts>...>);

    using Ts::operator()...;
  };

  template<typename... Ts>
  OverloadSet<Ts...>::OverloadSet(Ts... callables) noexcept(
    std::conjunction_v<std::is_nothrow_move_constructible<Ts>...>)
    : Ts(std::move(callables))... {}

  /**
   * Constructs an OverloadSet by decaying callable types.
   * @param callables The callables to combine.
   */
  template<typename... Ts>
  auto make_overload_set(Ts&&... callables) {
    return OverloadSet<std::decay_t<Ts>...>(std::forward<Ts>(callables)...);
  }

  /**
   * Applies the provided callables to a standard or Boost variant.
   * @param variant The variant to visit.
   * @param callables The callables to apply to the variant.
   */
  template<typename V, typename... Ts> requires
    IsSubclass<std::remove_cvref_t<V>, std::variant> ||
    IsSubclass<std::remove_cvref_t<V>, boost::variant>
  decltype(auto) visit(V&& variant, Ts&&... callables) {
    auto visitor = make_overload_set(std::forward<Ts>(callables)...);
    if constexpr(IsSubclass<std::remove_cvref_t<V>, std::variant>) {
      return std::visit(std::move(visitor), std::forward<V>(variant));
    } else {
      return boost::apply_visitor(std::move(visitor), std::forward<V>(variant));
    }
  }
}

#endif
