#ifndef BEAM_JSON_TYPES_HPP
#define BEAM_JSON_TYPES_HPP
#include <cstdint>
#include <type_traits>

namespace Beam {

  /**
   * Whether an integer is too wide to be represented exactly by a JSON number.
   * @tparam T The type to test.
   */
  template<typename T>
  constexpr auto is_wide_integer =
    std::is_integral_v<T> && sizeof(T) > sizeof(std::int32_t);
}

#endif
