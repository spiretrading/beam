#ifndef BEAM_SHAREDPOINTERPOLICY_HPP
#define BEAM_SHAREDPOINTERPOLICY_HPP
#include <memory>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /*! \class SharedPointerPolicy
      \brief Stores a pointer using an std::shared_ptr.
   */
  struct SharedPointerPolicy {
    template <typename T> struct apply { using type = std::shared_ptr<T>; };
  };
}

#endif
