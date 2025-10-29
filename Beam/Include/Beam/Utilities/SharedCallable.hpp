#ifndef BEAM_SHARED_CALLABLE_HPP
#define BEAM_SHARED_CALLABLE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps a move-only callable object into a copyable callable object.
   * @param F The type of callable object to wrap.
   */
  template<typename F>
  class SharedCallable {
    public:

      /** The type of callable object to wrap. */
      using Callable = dereference_t<F>;

      /**
       * Constructs a SharedCallable.
       * @param callable The callable to wrap.
       */
      template<typename FF>
      explicit SharedCallable(FF&& callable) noexcept(
        std::is_nothrow_constructible_v<F, FF&&>)
          requires std::constructible_from<F, FF&&>;

      /** Returns the wrapped callable. */
      auto& get_callable() noexcept;

      /** Returns the wrapped callable. */
      const auto& get_callable() const noexcept;

      /** Invokes the wrapped callable. */
      template<typename... Args>
      decltype(auto) operator ()(Args&&... args) const noexcept(noexcept(
          std::declval<const SharedCallable&>().get_callable()(
            std::forward<Args>(args)...))) {
        return get_callable()(std::forward<Args>(args)...);
      }

      /** Invokes the wrapped callable. */
      template<typename... Args>
      decltype(auto) operator ()(Args&&... args) noexcept(noexcept(
          std::declval<SharedCallable&>().get_callable()(
            std::forward<Args>(args)...))) {
        return get_callable()(std::forward<Args>(args)...);
      }

    private:
      std::conditional_t<std::is_copy_constructible_v<F>, local_ptr_t<F>,
        std::conditional_t<std::is_same_v<F, std::unique_ptr<Callable>>,
          std::shared_ptr<Callable>, std::shared_ptr<F>>> m_callable;
  };

  template<typename F>
  SharedCallable(F) -> SharedCallable<std::remove_cvref_t<F>>;

  template<typename F>
  template<typename FF>
  SharedCallable<F>::SharedCallable(FF&& callable) noexcept(
      std::is_nothrow_constructible_v<F, FF&&>) requires
        std::constructible_from<F, FF&&>
    : m_callable([&] {
        if constexpr(std::is_copy_constructible_v<F>) {
          return local_ptr_t<F>(std::forward<FF>(callable));
        } else if constexpr(std::is_same_v<F, std::unique_ptr<Callable>>) {
          return std::shared_ptr<Callable>(std::forward<FF>(callable));
        } else {
          return std::make_shared<F>(std::forward<FF>(callable));
        }
      }()) {}

  template<typename F>
  auto& SharedCallable<F>::get_callable() noexcept {
    return fully_dereference(m_callable);
  }

  template<typename F>
  const auto& SharedCallable<F>::get_callable() const noexcept {
    return fully_dereference(m_callable);
  }
}

#endif
