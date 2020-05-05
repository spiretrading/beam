#ifndef BEAM_LOCALPOINTERPOLICY_HPP
#define BEAM_LOCALPOINTERPOLICY_HPP
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /*! \class LocalPointerPolicy
      \brief Stores a pointer using a LocalPtr.
   */
  struct LocalPointerPolicy {
    template <typename T>
    struct apply {
      using type = LocalPtr<T>;
    };
  };
}

#endif
