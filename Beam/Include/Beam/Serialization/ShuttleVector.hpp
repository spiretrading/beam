#ifndef BEAM_SHUTTLE_VECTOR_HPP
#define BEAM_SHUTTLE_VECTOR_HPP
#include <vector>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T, typename A>
  constexpr auto is_structure<std::vector<T, A>> = false;

  template<typename T, typename A>
  struct Send<std::vector<T, A>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::vector<T, A>& value) const {
      sender.start_sequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        sender.send(i);
      }
      sender.end_sequence();
    }
  };

  template<typename T, typename A>
  struct Receive<std::vector<T, A>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::vector<T, A>& value) const {
      value.clear();
      auto size = int();
      receiver.start_sequence(name, size);
      value.reserve(size);
      for(auto i = 0; i < size; ++i) {
        value.push_back(receive<T>(receiver));
      }
      receiver.end_sequence();
    }
  };

  /**
   * Shuttles a vector and checks that no elements are <code>nullptr</code>.
   * @tparam S The type of DataShuttle to use.
   * @tparam T The type of value to shuttle.
   * @param shuttle The DataShuttle to use.
   * @param name The name of the value being shuttled.
   * @param value The value to shuttle.
   */
  template<IsShuttle S, typename T>
  void shuttle_non_null(S& shuttle, const char* name, std::vector<T>& value) {
    shuttle.shuttle(name, value);
    if constexpr(IsReceiver<S>) {
      for(auto& data : value) {
        if(!data) {
          boost::throw_with_location(
            SerializationException("Invalid null value."));
        }
      }
    }
  }
}

#endif
