#ifndef BEAM_TYPE_NOT_FOUND_EXCEPTION_HPP
#define BEAM_TYPE_NOT_FOUND_EXCEPTION_HPP
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam::Serialization {

  /** Signals that a polymorphic type was not registered. */
  class TypeNotFoundException : public SerializationException {
    public:

      /**
       * Constructs a TypeNotFoundException.
       * @param name The name of the type not found.
       */
      TypeNotFoundException(const std::string& name);
  };

  inline TypeNotFoundException::TypeNotFoundException(const std::string& name)
    : SerializationException("Type not found (" + name + ").") {}
}

#endif
