module;
#include "Prelude.hpp"

export module Beam:NullWriter;

export namespace Beam {

  /** A Writer whose data goes no where. */
  class NullWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);
  };

  template<IsConstBuffer B>
  void NullWriter::write(const B& data) {}
}

