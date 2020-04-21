#ifndef BEAM_CASTS_HPP
#define BEAM_CASTS_HPP
#include <memory>
#include <utility>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
  template<typename T>
  struct StaticCastConverter {
    template<typename U>
    constexpr decltype(auto) operator ()(U&& value) const {
      return static_cast<T>(std::forward<U>(value));
    }
  };

  template<typename T, typename U>
  T StaticCast(std::unique_ptr<U>&& source) {
    return T(static_cast<typename T::pointer>(source.release()));
  }
}

#endif
