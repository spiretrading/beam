#ifndef BEAM_UNIQUE_POINTER_POLICY_HPP
#define BEAM_UNIQUE_POINTER_POLICY_HPP
#include <memory>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /** Stores a pointer using an std::unique_ptr. */
  struct UniquePointerPolicy {
    template<typename T>
    struct apply {
      using type = std::unique_ptr<T>;
    };
  };
}

#endif
