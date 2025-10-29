#ifndef BEAM_SHUTTLE_CLONE_HPP
#define BEAM_SHUTTLE_CLONE_HPP
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleUniquePtr.hpp"

namespace Beam {

  /**
   * Constructs a clone of a potentially polymorphic object via serialization.
   * @param value The value to clone.
   * @param sender A Sender containing the TypeEntry for the value to clone.
   * @param receiver A Receiver containing the TypeEntry for the value to clone.
   * @return A clone of the <i>value</i> based on its serialization.
   */
  template<typename T, IsSender S>
  std::unique_ptr<T> shuttle_clone(
      const T& value, S& sender, inverse_t<S>& receiver) {
    auto buffer = typename S::Sink();
    sender.set(Ref(buffer));
    sender.send(&value);
    receiver.set(Ref(buffer));
    auto clone = std::unique_ptr<T>();
    receiver.receive(clone);
    return clone;
  }
}

#endif
