#ifndef BEAM_NULL_READER_HPP
#define BEAM_NULL_READER_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"

namespace Beam {
namespace IO {

  /** A Reader that contains no data. */
  class NullReader {
    public:
      bool IsDataAvailable() const;

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination, std::size_t size);
  };

  inline bool NullReader::IsDataAvailable() const {
    return false;
  }

  template<typename Buffer>
  std::size_t NullReader::Read(Out<Buffer> destination) {
    destination->Reset();
    return 0;
  }

  inline std::size_t NullReader::Read(char* destination, std::size_t size) {
    return 0;
  }

  template<typename Buffer>
  std::size_t NullReader::Read(Out<Buffer> destination, std::size_t size) {
    destination->Reset();
    return 0;
  }
}

  template<>
  struct ImplementsConcept<IO::NullReader, IO::Reader> : std::true_type {};
}

#endif
