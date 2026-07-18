module;
#include "Prelude.hpp"

export module Beam:UniquePointerPolicy;

export namespace Beam {

  /** Stores a pointer using an std::unique_ptr. */
  struct UniquePointerPolicy {
    template<typename T>
    struct apply {
      using type = std::unique_ptr<T>;
    };
  };
}

