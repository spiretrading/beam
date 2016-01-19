#ifndef BEAM_UNIQUEPOINTERPOLICY_HPP
#define BEAM_UNIQUEPOINTERPOLICY_HPP
#include <memory>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /*! \class UniquePointerPolicy
      \brief Stores a pointer using an std::unique_ptr.
   */
  struct UniquePointerPolicy {
    template<typename T>
    struct apply {
      typedef std::unique_ptr<T> type;
    };
  };
}

#endif
