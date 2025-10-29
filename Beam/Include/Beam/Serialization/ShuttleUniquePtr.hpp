#ifndef BEAM_SHUTTLE_UNIQUE_PTR_HPP
#define BEAM_SHUTTLE_UNIQUE_PTR_HPP
#include <memory>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T>
  constexpr auto is_structure<std::unique_ptr<T>> = false;

  template<typename T>
  struct Send<std::unique_ptr<T>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::unique_ptr<T>& value) const {
      sender.send(name, value.get());
    }
  };

  template<typename T>
  struct Receive<std::unique_ptr<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::unique_ptr<T>& value) const {
      auto proxy = static_cast<T*>(nullptr);
      receiver.receive(name, proxy);
      value.reset(proxy);
    }
  };
}

#endif
