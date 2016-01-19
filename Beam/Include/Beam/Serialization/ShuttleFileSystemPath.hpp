#ifndef BEAM_SHUTTLEFILESYSTEMPATH_HPP
#define BEAM_SHUTTLEFILESYSTEMPATH_HPP
#include <boost/filesystem/path.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<boost::filesystem::path> : std::false_type {};

  template<>
  struct Send<boost::filesystem::path> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::filesystem::path& value) const {
      shuttle.Send(name, value.string());
    }
  };

  template<>
  struct Receive<boost::filesystem::path> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::filesystem::path& value) const {
      std::string input;
      shuttle.Shuttle(name, input);
      value = boost::filesystem::path(input);
    }
  };
}
}

#endif
