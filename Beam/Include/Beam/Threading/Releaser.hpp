#ifndef BEAM_RELEASER_HPP
#define BEAM_RELEASER_HPP
#include <type_traits>
#include "Beam/Threading/LockRelease.hpp"

namespace Beam {

  /** Holds two lock releases, reacquiring the tail before the head. */
  template<typename T1, typename T2>
  struct Releaser {
    T1 m_head;
    T2 m_tail;
  };

  template<typename R1, typename R2>
  Releaser(R1&&, R2&&) ->
    Releaser<std::remove_cvref_t<R1>, std::remove_cvref_t<R2>>;

  /** Releases a lock until the returned guard is destroyed. */
  auto reverse_release(auto& head) {
    return release(head);
  }

  /**
   * Releases locks until the returned guard reacquires them in argument order.
   */
  auto reverse_release(auto& head, auto&... tail) {
    return Releaser(reverse_release(tail...), reverse_release(head));
  }
}

#endif
