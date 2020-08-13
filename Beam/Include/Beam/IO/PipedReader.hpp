#ifndef BEAM_PIPEDREADER_HPP
#define BEAM_PIPEDREADER_HPP
#include <climits>
#include <memory>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {
namespace IO {

  /*! \class PipedReader
      \brief Reads the contents written by a PipedWriter.
      \tparam BufferType The type of Buffer to read to.
   */
  template<typename BufferType>
  class PipedReader : private boost::noncopyable {
    public:
      using Buffer = BufferType;

      //! The type of the PipedWriter that connects to this PipedReader.
      using PipedWriter = IO::PipedWriter<Buffer>;

      //! Constructs an empty PipedReader.
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
  };

  template<typename BufferType>
  PipedReader<BufferType>::PipedReader()
    : m_reader(BufferFromString<Buffer>("")),
      m_messages(std::make_shared<Queue<BufferReader<Buffer>>>()) {}

  template<typename BufferType>
  PipedReader<BufferType>::~PipedReader() {
    m_messages->Break(EndOfFileException("Pipe broken."));
  }

  template<typename BufferType>
  bool PipedReader<BufferType>::IsDataAvailable() const {
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

  template<typename BufferType>
  std::size_t PipedReader<BufferType>::Read(Out<Buffer> destination) {
    return Read(Store(destination), INT_MAX);
  }

  template<typename BufferType>
  std::size_t PipedReader<BufferType>::Read(char* destination,
      std::size_t size) {
    while(true) {
      try {
        return m_reader.Read(destination, size);
      } catch(const EndOfFileException&) {
        m_reader = m_messages->Pop();
      }
    }
  }

  template<typename BufferType>
  std::size_t PipedReader<BufferType>::Read(Out<Buffer> destination,
      std::size_t size) {
    while(true) {
      try {
        return m_reader.Read(Store(destination), size);
      } catch(const EndOfFileException&) {
        m_reader = m_messages->Pop();
      }
    }
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::PipedReader<BufferType>,
    IO::Reader<BufferType>> : std::true_type {};
}

#endif
