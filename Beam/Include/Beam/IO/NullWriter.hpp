#ifndef BEAM_NULL_WRITER_HPP
#define BEAM_NULL_WRITER_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"

namespace Beam {
namespace IO {

  /** A Writer whose data goes no where. */
  class NullWriter {
    public:
      using Buffer = SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename B>
      void Write(const B& data);
  };

  inline void NullWriter::Write(const void* data, std::size_t size) {}

  template<typename B>
  void NullWriter::Write(const B& data) {}
}

  template<typename Buffer>
  struct ImplementsConcept<IO::NullWriter, IO::Writer<Buffer>> :
    std::true_type {};
}

#endif
