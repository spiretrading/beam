#ifndef BEAM_CASTS_HPP
#define BEAM_CASTS_HPP
#include <memory>
#include <utility>

namespace Beam {

  /**
   * Casts a std::unique_ptr<U> to a std::unique_ptr<T> using static_cast.
   * @param p The pointer to cast.
   * @return The cast pointer.
   */
  template<typename T, typename U>
  std::unique_ptr<T> static_pointer_cast(std::unique_ptr<U>&& p) {
    return std::unique_ptr<T>(static_cast<T*>(p.release()));
  }
}

#endif
