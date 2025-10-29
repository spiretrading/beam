#ifndef BEAM_SHUTTLE_SHARED_PTR_HPP
#define BEAM_SHUTTLE_SHARED_PTR_HPP
#include <memory>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T>
  constexpr auto is_structure<std::shared_ptr<T>> = false;

  template<typename T>
  struct Send<std::shared_ptr<T>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::shared_ptr<T>& value) const {
      sender.send(name, value.get());
    }
  };

  template<typename T>
  struct Receive<std::shared_ptr<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::shared_ptr<T>& value) const {
      value.reset(receive<T*>(receiver, name));
    }
  };
}

#endif
