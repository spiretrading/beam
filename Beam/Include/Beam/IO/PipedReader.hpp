#ifndef BEAM_PIPED_READER_HPP
#define BEAM_PIPED_READER_HPP
#include <memory>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /** Reads the contents written by a PipedWriter. */
  class PipedReader {
    public:

      /** Constructs an empty PipedReader. */
      PipedReader();

      ~PipedReader();

      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);

    private:
      friend class PipedWriter;
      mutable BufferReader<SharedBuffer> m_reader;
      std::shared_ptr<Queue<BufferReader<SharedBuffer>>> m_messages;

      PipedReader(const PipedReader&) = delete;
      PipedReader& operator =(const PipedReader&) = delete;
  };

  inline PipedReader::PipedReader()
    : m_reader(SharedBuffer()),
      m_messages(std::make_shared<Queue<BufferReader<SharedBuffer>>>()) {}

  inline PipedReader::~PipedReader() {
    m_messages->close(EndOfFileException("Pipe broken."));
  }

  inline bool PipedReader::poll() const {
    while(true) {
      if(m_reader.poll()) {
        return true;
      }
      if(auto reader = m_messages->try_pop()) {
        m_reader = std::move(*reader);
      } else {
        return false;
      }
    }
    return false;
  }

  template<IsBuffer R>
  std::size_t PipedReader::read(Out<R> destination, std::size_t size) {
    while(true) {
      try {
        return m_reader.read(out(destination), size);
      } catch(const EndOfFileException&) {
        m_reader = m_messages->pop();
      }
    }
  }

  inline PipedWriter::PipedWriter(Ref<PipedReader> destination)
    : m_messages(destination->m_messages) {}

  inline PipedWriter::~PipedWriter() {
    close();
  }

  inline void PipedWriter::close(const std::exception_ptr& e) {
    m_messages->close(e);
  }

  template<typename E>
  void PipedWriter::close(const E& e) {
    close(std::make_exception_ptr(e));
  }

  inline void PipedWriter::close() {
    close(EndOfFileException("Pipe broken."));
  }

  template<IsConstBuffer T>
  void PipedWriter::write(const T& data) {
    m_messages->push(BufferReader(SharedBuffer(data)));
  }
}

#endif
