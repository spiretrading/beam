#ifndef BEAM_SHUTTLE_CLONE_HPP
#define BEAM_SHUTTLE_CLONE_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/ShuttleUniquePtr.hpp"

namespace Beam {

  /**
   * Constructs a value clone via binary serialization.
   * @tparam T The type of value to clone.
   * @param value The value to clone.
   * @return An independent clone of the serialized value.
   */
  template<typename T>
  T shuttle_clone(const T& value) {
    using Value = T;
    auto buffer = SharedBuffer();
    auto sender = BinarySender<SharedBuffer>();
    sender.set(Ref(buffer));
    sender.send(value);
    auto receiver = BinaryReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    return receive<Value>(receiver);
  }

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
