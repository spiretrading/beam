#ifndef BEAM_SHUTTLEUUID_HPP
#define BEAM_SHUTTLEUUID_HPP
#include <sstream>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<boost::uuids::uuid> : std::false_type {};

  template<>
  struct Send<boost::uuids::uuid> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::uuids::uuid& value) const {
      std::stringstream sink;
      sink << value;
      shuttle.Send(name, sink.str());
    }
  };

  template<>
  struct Receive<boost::uuids::uuid> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::uuids::uuid& value) const {
      std::string input;
      shuttle.Shuttle(name, input);
      value = boost::uuids::string_generator()(input);
    }
  };
}
}

#endif
