#ifndef BEAM_SHARED_POINTER_POLICY_HPP
#define BEAM_SHARED_POINTER_POLICY_HPP
#include <memory>

namespace Beam {

  /** Stores a pointer using an std::shared_ptr. */
  struct SharedPointerPolicy {
    template<typename T>
    struct apply {
      using type = std::shared_ptr<T>;
    };
  };
}

#endif
