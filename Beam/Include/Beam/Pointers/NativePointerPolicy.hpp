#ifndef BEAM_NATIVEPOINTERPOLICY_HPP
#define BEAM_NATIVEPOINTERPOLICY_HPP
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /*! \class NativePointerPolicy
      \brief Stores a pointer using a native pointer type.
   */
  struct NativePointerPolicy {
    template <typename T>
    struct apply {
      using type = T*;
    };
  };
}

#endif
