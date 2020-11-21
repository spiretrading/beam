#ifndef BEAM_PIPED_READER_HPP
#define BEAM_PIPED_READER_HPP
#include <limits>
#include <memory>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {
namespace IO {

  /**
   * Reads the contents written by a PipedWriter.
   * @param <B> The type of Buffer to read to.
   */
  template<typename B>
  class PipedReader {
    public:
      using Buffer = B;

      /** The type of the PipedWriter that connects to this PipedReader. */
      using PipedWriter = IO::PipedWriter<Buffer>;

      /** Constructs an empty PipedReader. */
      PipedReader();

      ~PipedReader();

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      friend class IO::PipedWriter<Buffer>;
      mutable BufferReader<Buffer> m_reader;
      std::shared_ptr<Queue<BufferReader<Buffer>>> m_messages;

      PipedReader(const PipedReader&) = delete;
      PipedReader& operator =(const PipedReader&) = delete;
  };

  template<typename B>
  PipedReader<B>::PipedReader()
    : m_reader(BufferFromString<Buffer>("")),
      m_messages(std::make_shared<Queue<BufferReader<Buffer>>>()) {}

  template<typename B>
  PipedReader<B>::~PipedReader() {
    m_messages->Break(EndOfFileException("Pipe broken."));
  }

  template<typename B>
  bool PipedReader<B>::IsDataAvailable() const {
    while(true) {
      if(m_reader.IsDataAvailable()) {
        return true;
      }
      if(auto reader = m_messages->TryPop()) {
        m_reader = std::move(*reader);
      } else {
        return false;
      }
    }
    return false;
  }

  template<typename B>
  std::size_t PipedReader<B>::Read(Out<Buffer> destination) {
    return Read(Store(destination), std::numeric_limits<std::size_t>::max());
  }

  template<typename B>
  std::size_t PipedReader<B>::Read(char* destination, std::size_t size) {
    while(true) {
      try {
        return m_reader.Read(destination, size);
      } catch(const EndOfFileException&) {
        m_reader = m_messages->Pop();
      }
    }
  }

  template<typename B>
  std::size_t PipedReader<B>::Read(Out<Buffer> destination, std::size_t size) {
    while(true) {
      try {
        return m_reader.Read(Store(destination), size);
      } catch(const EndOfFileException&) {
        m_reader = m_messages->Pop();
      }
    }
  }
}

  template<typename B>
  struct ImplementsConcept<IO::PipedReader<B>, IO::Reader<B>> :
    std::true_type {};
}

#endif
