#ifndef BEAM_SHUTTLE_VARIANT_HPP
#define BEAM_SHUTTLE_VARIANT_HPP
#include <utility>
#include <variant>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam {
namespace Details {
  template<IsSender S, typename... Ts, std::size_t... Is>
  void send(S& sender, int which, const std::variant<Ts...>& value,
      std::index_sequence<Is...>) {
    auto handled = ((which == static_cast<int>(Is) && [&] {
      sender.send("value", std::get<Is>(value));
      return true;
    }()) || ...);
    if(!handled) {
      boost::throw_with_location(SerializationException("Invalid variant."));
    }
  }

  template<IsReceiver R, typename... Ts, std::size_t... Is>
  void receive(R& receiver, int which, std::variant<Ts...>& value,
      std::index_sequence<Is...>) {
    auto handled = ((which == static_cast<int>(Is) && [&] {
      using Type = std::variant_alternative_t<Is, std::variant<Ts...>>;
      auto received = Type();
      receiver.receive("value", received);
      value.template emplace<Is>(std::move(received));
      return true;
    }()) || ...);
    if(!handled) {
      boost::throw_with_location(SerializationException("Invalid variant."));
    }
  }
}

  template<typename T>
  struct Send<std::variant<T>> {
    template<IsSender S>
    void operator ()(
        S& sender, const std::variant<T>& value, unsigned int version) const {
      sender.send("value", std::get<0>(value));
    }
  };

  template<typename... Ts>
  struct Send<std::variant<Ts...>> {
    template<IsSender S>
    void operator ()(S& sender, const std::variant<Ts...>& value,
        unsigned int version) const {
      auto which = static_cast<int>(value.index());
      sender.send("which", which);
      Details::send(
        sender, which, value, std::make_index_sequence<sizeof...(Ts)>());
    }
  };

  template<typename T>
  struct Receive<std::variant<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, std::variant<T>& value, unsigned int version) const {
      receiver.receive("value", std::get<0>(value));
    }
  };

  template<typename... Ts>
  struct Receive<std::variant<Ts...>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, std::variant<Ts...>& value, unsigned int version) const {
      auto which = int();
      receiver.receive("which", which);
      Details::receive(
        receiver, which, value, std::make_index_sequence<sizeof...(Ts)>());
    }
  };
}

#endif
