#ifndef BEAM_NULLREADER_HPP
#define BEAM_NULLREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"

namespace Beam {
namespace IO {

  /*  \class NullReader
      \brief A Reader that contains no data.
   */
  class NullReader : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      bool IsDataAvailable() const;

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination, std::size_t size);
  };

  inline bool NullReader::IsDataAvailable() const {
    return false;
  }

  template<typename BufferType>
  std::size_t NullReader::Read(Out<BufferType> destination) {
    destination->Reset();
    return 0;
  }

  inline std::size_t NullReader::Read(char* destination, std::size_t size) {
    return 0;
  }

  template<typename BufferType>
  std::size_t NullReader::Read(Out<BufferType> destination, std::size_t size) {
    destination->Reset();
    return 0;
  }
}

  template<typename Buffer>
  struct ImplementsConcept<IO::NullReader, IO::Reader<Buffer>> :
    std::true_type {};
}

#endif
