#ifndef BEAM_SHUTTLE_ATOMIC_HPP
#define BEAM_SHUTTLE_ATOMIC_HPP
#include <atomic>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T>
  constexpr auto is_structure<std::atomic<T>> = false;

  template<typename T>
  struct Send<std::atomic<T>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::atomic<T>& value) const {
      sender.send(name, value.load());
    }
  };

  template<typename T>
  struct Receive<std::atomic<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::atomic<T>& value) const {
      value.store(receive<T>(receiver, name));
    }
  };
}

#endif
