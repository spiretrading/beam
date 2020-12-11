#ifndef BEAM_CONST_POINTER_POLICY_HPP
#define BEAM_CONST_POINTER_POLICY_HPP
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /**
   * Takes a pointer policy and const qualifies the type it evaluates to.
   * @param <P> The pointer policy to const qualify.
   */
  template<typename P>
  struct ConstPointerPolicy {

    /** The pointer policy to const qualify. */
    using Policy = P;

    template<typename T>
    struct apply {
      using type = typename Policy::template apply<const T>::type;
    };
  };
}

#endif
