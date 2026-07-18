module;
#include "Prelude.hpp"

export module Beam:ShuttleUuid;

export namespace Beam {
  template<>
  constexpr auto is_structure<boost::uuids::uuid> = false;

  template<>
  struct Send<boost::uuids::uuid> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const boost::uuids::uuid& value) const {
      auto sink = std::stringstream();
      sink << value;
      sender.send(name, sink.str());
    }
  };

  template<>
  struct Receive<boost::uuids::uuid> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, boost::uuids::uuid& value) const {
      value =
        boost::uuids::string_generator()(receive<std::string>(receiver, name));
    }
  };
}

