#ifndef BEAM_SHUTTLE_TUPLE_HPP
#define BEAM_SHUTTLE_TUPLE_HPP
#include <tuple>
#include <utility>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam::Serialization {
namespace Details {
  template<typename Shuttler, typename Tuple>
  void Send(Shuttler& shuttle, std::integer_sequence<std::size_t>,
    const Tuple& value) {}

  template<typename Shuttler, typename Tuple, std::size_t Head,
    std::size_t... Tail>
  void Send(Shuttler& shuttle,
      std::integer_sequence<std::size_t, Head, Tail...>, const Tuple& value) {
    shuttle.Shuttle(std::get<Head>(value));
    Send(shuttle, std::integer_sequence<std::size_t, Tail...>(), value);
  }

  template<typename Shuttler, typename Tuple>
  void Receive(Shuttler& shuttle, std::integer_sequence<std::size_t>,
    Tuple& value) {}

  template<typename Shuttler, typename Tuple, std::size_t Head,
    std::size_t... Tail>
  void Receive(Shuttler& shuttle,
      std::integer_sequence<std::size_t, Head, Tail...>, Tuple& value) {
    shuttle.Shuttle(std::get<Head>(value));
    Receive(shuttle, std::integer_sequence<std::size_t, Tail...>(), value);
  }
}

  template<typename... Args>
  struct IsStructure<std::tuple<Args...>> : std::false_type {};

  template<typename... Args>
  struct Send<std::tuple<Args...>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::tuple<Args...>& value) const {
      using Sequence = std::make_integer_sequence<std::size_t, sizeof...(Args)>;
      shuttle.StartSequence(name);
      Details::Send(shuttle, Sequence(), value);
      shuttle.EndSequence();
    }
  };

  template<typename... Args>
  struct Receive<std::tuple<Args...>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::tuple<Args...>& value) const {
      using Sequence = std::make_integer_sequence<std::size_t, sizeof...(Args)>;
      shuttle.StartSequence(name);
      Details::Receive(shuttle, Sequence(), value);
      shuttle.EndSequence();
    }
  };
}

#endif
