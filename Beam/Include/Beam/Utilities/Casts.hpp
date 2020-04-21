#ifndef BEAM_CASTS_HPP
#define BEAM_CASTS_HPP
#include <memory>
#include <utility>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  //! Implements a callable object performing a static_cast.
  template<typename T>
  struct StaticCastConverter {

    //! Performs a static_cast.
    template<typename U>
    constexpr decltype(auto) operator ()(U&& value) const {
      return static_cast<T>(std::forward<U>(value));
    }
  };

  //! Performs a static_cast on a unique_ptr.
  template<typename T, typename U>
  T StaticCast(std::unique_ptr<U>&& source) {
    return T(static_cast<typename T::pointer>(source.release()));
  }
}

#endif
