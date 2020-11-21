#ifndef BEAM_PIPED_WRITER_HPP
#define BEAM_PIPED_WRITER_HPP
#include <memory>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {
namespace IO {

  /** Writes to a PipedReader. */
  template<typename B>
  class PipedWriter {
    public:
      using Buffer = B;

      /** The type of the PipedReader that this writer connects to. */
      using PipedReader = IO::PipedReader<Buffer>;

      /**
       * Constructs a PipedWriter.
       * @param destination The PipedReader to connect to.
       */
      PipedWriter(Ref<PipedReader> destination);

      ~PipedWriter();

      /**
       * Breaks the pipe.
       * @param e The cause of the break.
       */
      void Break(const std::exception_ptr& e);

      /**
       * Breaks the pipe.
       * @param e The cause of the break.
       */
      template<typename E>
      void Break(const E& e);

      /** Breaks the pipe. */
      void Break();

      void Write(const void* data, std::size_t size);

      void Write(const Buffer& data);

    private:
      std::shared_ptr<Queue<BufferReader<Buffer>>> m_messages;

      PipedWriter(const PipedWriter&) = delete;
      PipedWriter& operator =(const PipedWriter&) = delete;
  };

  template<typename B>
  PipedWriter<B>::PipedWriter(Ref<PipedReader> destination)
    : m_messages(destination->m_messages) {}

  template<typename B>
  PipedWriter<B>::~PipedWriter() {
    Break();
  }

  template<typename B>
  void PipedWriter<B>::Break(const std::exception_ptr& e) {
    m_messages->Break(e);
  }

  template<typename B>
  template<typename E>
  void PipedWriter<B>::Break(const E& e) {
    Break(std::make_exception_ptr(e));
  }

  template<typename B>
  void PipedWriter<B>::Break() {
    Break(EndOfFileException("Pipe broken."));
  }

  template<typename B>
  void PipedWriter<B>::Write(const void* data, std::size_t size) {
    Write(Buffer(data, size));
  }

  template<typename B>
  void PipedWriter<B>::Write(const Buffer& data) {
    m_messages->Push(BufferReader(data));
  }
}

  template<typename B>
  struct ImplementsConcept<IO::PipedWriter<B>,
    IO::Writer<typename IO::PipedWriter<B>::Buffer>> : std::true_type {};
}

#endif
