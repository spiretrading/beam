#ifndef BEAM_SHUTTLEVECTOR_HPP
#define BEAM_SHUTTLEVECTOR_HPP
#include <vector>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T, typename A>
  struct IsStructure<std::vector<T, A>> : std::false_type {};

  template<typename T, typename A>
  struct Send<std::vector<T, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::vector<T, A>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename T, typename A>
  struct Receive<std::vector<T, A>, typename std::enable_if<
      IsDefaultConstructable<T>::value>::type> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::vector<T, A>& value) const {
      value.clear();
      auto size = int();
      shuttle.StartSequence(name, size);
      for(auto i = 0; i < size; ++i) {
        value.emplace_back();
        shuttle.Shuttle(value.back());
      }
      shuttle.EndSequence();
    }
  };

  template<typename T, typename A>
  struct Receive<std::vector<T, A>, typename std::enable_if<
      !IsDefaultConstructable<T>::value>::type> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::vector<T, A>& value) const {
      value.clear();
      auto size = int();
      shuttle.StartSequence(name, size);
      for(auto i = 0; i < size; ++i) {
        value.push_back(static_cast<T>(ReceiveBuilder()));
        shuttle.Shuttle(value.back());
      }
      shuttle.EndSequence();
    }
  };

  //! Shuttles a vector and checks that no elements are <code>nullptr</code>.
  /*!
    \tparam Shuttler The type of DataShuttle to use.
    \tparam T The type of value to shuttle.
    \param shuttle The DataShuttle to use.
    \param name The name of the value being shuttled.
    \param value The value to shuttle.
  */
  template<typename Shuttler, typename T>
  void ShuttleNonNull(Shuttler& shuttle, const char* name,
      std::vector<T>& value) {
    shuttle.Shuttle(name, value);
    if(IsReceiver<Shuttler>::value) {
      for(auto& data : value) {
        if(data == nullptr) {
          BOOST_THROW_EXCEPTION(SerializationException("Invalid null value."));
        }
      }
    }
  }
}
}

#endif
