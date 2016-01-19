#ifndef BEAM_NULLSLOT_HPP
#define BEAM_NULLSLOT_HPP
#include "Beam/SignalHandling/SignalHandling.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \struct NullSlot
      \brief Defines a generic slot that does nothing.
   */
  struct NullSlot {

    template<typename... Args>
    void operator ()(Args&&... args) const;
  };

  template<typename... Args>
  void NullSlot::operator ()(Args&&... args) const {}
}
}

#endif
