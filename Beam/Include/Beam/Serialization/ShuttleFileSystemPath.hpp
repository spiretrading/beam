#ifndef BEAM_SHUTTLE_FILE_SYSTEM_PATH_HPP
#define BEAM_SHUTTLE_FILE_SYSTEM_PATH_HPP
#include <filesystem>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<>
  constexpr auto is_structure<std::filesystem::path> = false;

  template<>
  struct Send<std::filesystem::path> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::filesystem::path& value) const {
      sender.send(name, value.string());
    }
  };

  template<>
  struct Receive<std::filesystem::path> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::filesystem::path& value) const {
      value = receive<std::string>(receiver, name);
    }
  };
}

#endif
