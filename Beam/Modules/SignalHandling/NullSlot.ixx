module;
#include "Prelude.hpp"

export module Beam:NullSlot;

export namespace Beam {

  /** A slot that does nothing. */
  struct NullSlot {

    /**
     * Invokes the slot.
     * @param args The arguments to pass to the slot
     */
    template<typename... Args>
    constexpr void operator ()(Args&&... args) const noexcept;
  };

  template<typename... Args>
  constexpr void NullSlot::operator ()(Args&&... args) const noexcept {}
}
