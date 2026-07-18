module;
#include "Prelude.hpp"

export module Beam:ShuttleTuple;

import :Value;

export namespace Beam {
  template<typename... Args>
  constexpr auto is_structure<std::tuple<Args...>> = false;

  template<typename... Args>
  struct Send<std::tuple<Args...>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::tuple<Args...>& value) const {
      sender.start_sequence(name);
      std::apply([&] (const auto&... elements) {
        ((sender.send(elements)), ...);
      }, value);
      sender.end_sequence();
    }
  };

  template<typename... Args>
  struct Receive<std::tuple<Args...>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::tuple<Args...>& value) const {
      receiver.start_sequence(name);
      std::apply([&] (auto&... elements) {
        ((receiver.receive(elements)), ...);
      }, value);
      receiver.end_sequence();
    }
  };
}

