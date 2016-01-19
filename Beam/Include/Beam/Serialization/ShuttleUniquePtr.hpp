#ifndef BEAM_SHUTTLEUNIQUEPTR_HPP
#define BEAM_SHUTTLEUNIQUEPTR_HPP
#include <memory>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T>
  struct IsStructure<std::unique_ptr<T>> : std::false_type {};

  template<typename T>
  struct Send<std::unique_ptr<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::unique_ptr<T>& value) const {
      shuttle.Send(name, value.get());
    }
  };

  template<typename T>
  struct Receive<std::unique_ptr<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::unique_ptr<T>& value) const {
      T* proxy;
      shuttle.Shuttle(name, proxy);
      value.reset(proxy);
    }
  };
}
}

#endif
