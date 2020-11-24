#ifndef BEAM_LOCAL_POINTER_POLICY_HPP
#define BEAM_LOCAL_POINTER_POLICY_HPP
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /** Stores a pointer using a LocalPtr. */
  struct LocalPointerPolicy {
    template <typename T>
    struct apply {
      using type = LocalPtr<T>;
    };
  };
}

#endif
