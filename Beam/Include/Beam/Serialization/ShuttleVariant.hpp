#ifndef BEAM_SHUTTLE_VARIANT_HPP
#define BEAM_SHUTTLE_VARIANT_HPP
#include <utility>
#include <variant>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/throw_exception.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam {
namespace Details {
  template<IsSender S, typename V, std::size_t... Is>
  void send(S& sender, int which, const V& value, std::index_sequence<Is...>) {
    auto handled = ((which == static_cast<int>(Is) && [&] {
      using Type = typename boost::mpl::at_c<typename V::types, Is>::type;
      sender.send("value", boost::get<Type>(value));
      return true;
    }()) || ...);
    if(!handled) {
      boost::throw_with_location(SerializationException("Invalid variant."));
    }
  }

  template<IsReceiver R, typename V, std::size_t... Is>
  void receive(R& receiver, int which, V& value, std::index_sequence<Is...>) {
    auto handled = ((which == static_cast<int>(Is) && [&] {
      using Type = typename boost::mpl::at_c<typename V::types, Is>::type;
      auto received = Type();
      receiver.receive("value", received);
      value = std::move(received);
      return true;
    }()) || ...);
    if(!handled) {
      boost::throw_with_location(SerializationException("Invalid variant."));
    }
  }
}

  template<typename T>
  struct Send<boost::variant<T>> {
    template<IsSender S>
    void operator ()(
        S& sender, const boost::variant<T>& value, unsigned int version) const {
      sender.send("value", boost::get<T>(value));
    }
  };

  template<typename Arg, typename... Args>
  struct Send<boost::variant<Arg, Args...>> {
    template<IsSender S>
    void operator ()(S& sender,
        const boost::variant<Arg, Args...>& value, unsigned int version) const {
      using Variant = boost::variant<Arg, Args...>;
      using Sequence = std::make_index_sequence<
        boost::mpl::size<typename Variant::types>::value>;
      auto which = value.which();
      sender.send("which", which);
      Details::send(sender, which, value, Sequence());
    }
  };

  template<typename T>
  struct Receive<boost::variant<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, boost::variant<T>& value, unsigned int version) const {
      receiver.receive("value", boost::get<T>(value));
    }
  };

  template<typename Arg, typename... Args>
  struct Receive<boost::variant<Arg, Args...>> {
    template<IsReceiver R>
    void operator ()(R& receiver, boost::variant<Arg, Args...>& value,
        unsigned int version) const {
      using Variant = boost::variant<Arg, Args...>;
      using Sequence = std::make_index_sequence<
        boost::mpl::size<typename Variant::types>::value>;
      auto which = int();
      receiver.receive("which", which);
      Details::receive(receiver, which, value, Sequence());
    }
  };

namespace Details {
  template<IsSender S, typename... Ts, std::size_t... Is>
  void send_std(S& sender, int which, const std::variant<Ts...>& value,
      std::index_sequence<Is...>) {
    auto handled = ((which == static_cast<int>(Is) && [&] {
      using Type = std::variant_alternative_t<Is, std::variant<Ts...>>;
      sender.send("value", std::get<Type>(value));
      return true;
    }()) || ...);
    if(!handled) {
      boost::throw_with_location(SerializationException("Invalid variant."));
    }
  }

  template<IsReceiver R, typename... Ts, std::size_t... Is>
  void receive_std(R& receiver, int which, std::variant<Ts...>& value,
      std::index_sequence<Is...>) {
    auto handled = ((which == static_cast<int>(Is) && [&] {
      using Type = std::variant_alternative_t<Is, std::variant<Ts...>>;
      auto received = Type();
      receiver.receive("value", received);
      value = std::move(received);
      return true;
    }()) || ...);
    if(!handled) {
      boost::throw_with_location(SerializationException("Invalid variant."));
    }
  }
}

  template<typename... Ts>
  struct Send<std::variant<Ts...>> {
    template<IsSender S>
    void operator ()(S& sender, const std::variant<Ts...>& value,
        unsigned int version) const {
      auto which = static_cast<int>(value.index());
      sender.send("which", which);
      Details::send_std(
        sender, which, value, std::make_index_sequence<sizeof...(Ts)>());
    }
  };

  template<typename... Ts>
  struct Receive<std::variant<Ts...>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, std::variant<Ts...>& value, unsigned int version) const {
      auto which = int();
      receiver.receive("which", which);
      Details::receive_std(
        receiver, which, value, std::make_index_sequence<sizeof...(Ts)>());
    }
  };
}

#endif
