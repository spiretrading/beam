#ifndef BEAM_NATIVE_POINTER_POLICY_HPP
#define BEAM_NATIVE_POINTER_POLICY_HPP
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /** Stores a pointer using a native pointer type. */
  struct NativePointerPolicy {
    template<typename T>
    struct apply {
      using type = T*;
    };
  };
}

#endif
