#ifndef BEAM_NULL_WRITER_HPP
#define BEAM_NULL_WRITER_HPP
#include "Beam/IO/Writer.hpp"

namespace Beam {

  /** A Writer whose data goes no where. */
  class NullWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);
  };

  template<IsConstBuffer B>
  void NullWriter::write(const B& data) {}
}

#endif
