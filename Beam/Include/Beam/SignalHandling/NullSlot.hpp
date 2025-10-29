#ifndef BEAM_NULL_SLOT_HPP
#define BEAM_NULL_SLOT_HPP

namespace Beam {

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

#endif
