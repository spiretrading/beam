module;
#include "Prelude.hpp"

export module Beam:ShuttleAtomic;

export namespace Beam {
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

