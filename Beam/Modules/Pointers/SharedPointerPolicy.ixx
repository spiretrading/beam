module;
#include "Prelude.hpp"

export module Beam:SharedPointerPolicy;

export namespace Beam {

  /** Stores a pointer using an std::shared_ptr. */
  struct SharedPointerPolicy {
    template<typename T>
    struct apply {
      using type = std::shared_ptr<T>;
    };
  };
}

