#ifndef BEAM_SHUTTLETUPLE_HPP
#define BEAM_SHUTTLETUPLE_HPP
#include <tuple>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"

namespace Beam {
namespace Serialization {
namespace Details {
  template<typename Shuttler, typename Tuple>
  void Send(Shuttler& shuttle, IntegerSequence<>, const Tuple& value) {}

  template<typename Shuttler, typename Tuple, int Head, int... Tail>
  void Send(Shuttler& shuttle, IntegerSequence<Head, Tail...>,
      const Tuple& value) {
    shuttle.Shuttle(std::get<Head>(value));
    Send(shuttle, IntegerSequence<Tail...>(), value);
  }

  template<typename Shuttler, typename Tuple>
  void Receive(Shuttler& shuttle, IntegerSequence<>, Tuple& value) {}

  template<typename Shuttler, typename Tuple, int Head, int... Tail>
  void Receive(Shuttler& shuttle, IntegerSequence<Head, Tail...>,
      Tuple& value) {
    shuttle.Shuttle(std::get<Head>(value));
    Receive(shuttle, IntegerSequence<Tail...>(), value);
  }
}

  template<typename... Args>
  struct IsStructure<std::tuple<Args...>> : std::false_type {};

  template<typename... Args>
  struct Send<std::tuple<Args...>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::tuple<Args...>& value) const {
      using Sequence = typename IntegerSequenceGenerator<sizeof...(Args)>::type;
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
      using Sequence = typename IntegerSequenceGenerator<sizeof...(Args)>::type;
      shuttle.StartSequence(name);
      Details::Receive(shuttle, Sequence(), value);
      shuttle.EndSequence();
    }
  };
}
}

#endif
