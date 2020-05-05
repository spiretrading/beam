#ifndef BEAM_NULLWRITER_HPP
#define BEAM_NULLWRITER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"

namespace Beam {
namespace IO {

  /*  \class NullWriter
      \brief A Writer whose data goes no where.
   */
  class NullWriter : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);
  };

  inline void NullWriter::Write(const void* data, std::size_t size) {}

  template<typename BufferType>
  void NullWriter::Write(const BufferType& data) {}
}

  template<typename Buffer>
  struct ImplementsConcept<IO::NullWriter, IO::Writer<Buffer>> :
    std::true_type {};
}

#endif
