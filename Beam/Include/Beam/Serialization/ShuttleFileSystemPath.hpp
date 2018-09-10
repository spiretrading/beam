#ifndef BEAM_SHUTTLEFILESYSTEMPATH_HPP
#define BEAM_SHUTTLEFILESYSTEMPATH_HPP
#include <filesystem>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<std::filesystem::path> : std::false_type {};

  template<>
  struct Send<std::filesystem::path> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::filesystem::path& value) const {
      shuttle.Send(name, value.string());
    }
  };

  template<>
  struct Receive<std::filesystem::path> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::filesystem::path& value) const {
      std::string input;
      shuttle.Shuttle(name, input);
      value = std::filesystem::path(input);
    }
  };
}
}

#endif
