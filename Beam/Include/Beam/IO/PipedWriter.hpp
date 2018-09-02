#ifndef BEAM_PIPEDWRITER_HPP
#define BEAM_PIPEDWRITER_HPP
#include <memory>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {
namespace IO {

  /*! \class PipedWriter
      \brief Writes to a PipedReader.
   */
  template<typename BufferType>
  class PipedWriter : private boost::noncopyable {
    public:
      typedef BufferType Buffer;

      //! The type of the PipedReader that this writer connects to.
      typedef IO::PipedReader<Buffer> PipedReader;

      //! Constructs a PipedWriter.
      /*!
        \param destination The PipedReader to connect to.
      */
      PipedWriter(Ref<PipedReader> destination);

      ~PipedWriter();

      //! Breaks the pipe.
      /*!
        \param e The cause of the break.
      */
      void Break(const std::exception_ptr& e);

      //! Breaks the pipe.
      /*!
        \param e The cause of the break.
      */
      template<typename E>
      void Break(const E& e);

      //! Breaks the pipe.
      void Break();

      void Write(const void* data, std::size_t size);

      void Write(const Buffer& data);

    private:
      std::shared_ptr<Queue<BufferReader<Buffer>>> m_messages;
  };

  template<typename BufferType>
  PipedWriter<BufferType>::PipedWriter(Ref<PipedReader> destination)
      : m_messages(destination->m_messages) {}

  template<typename BufferType>
  PipedWriter<BufferType>::~PipedWriter() {
    Break();
  }

  template<typename BufferType>
  void PipedWriter<BufferType>::Break(const std::exception_ptr& e) {
    m_messages->Break(e);
  }

  template<typename BufferType>
  template<typename E>
  void PipedWriter<BufferType>::Break(const E& e) {
    Break(std::make_exception_ptr(e));
  }

  template<typename BufferType>
  void PipedWriter<BufferType>::Break() {
    Break(EndOfFileException("Pipe broken."));
  }

  template<typename BufferType>
  void PipedWriter<BufferType>::Write(const void* data, std::size_t size) {
    Write(Buffer(data, size));
  }

  template<typename BufferType>
  void PipedWriter<BufferType>::Write(const Buffer& data) {
    m_messages->Push(BufferReader<Buffer>(data));
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::PipedWriter<BufferType>,
    IO::Writer<typename IO::PipedWriter<BufferType>::Buffer>> :
    std::true_type {};
}

#endif
