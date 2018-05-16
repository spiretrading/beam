#ifndef BEAM_QUEUEDREADER_HPP
#define BEAM_QUEUEDREADER_HPP
#include <atomic>
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {
namespace IO {

  /*! \class QueuedReader
      \brief Continuously reads data into a queue, asynchronously.
      \tparam BufferType The type of Buffer to read into.
      \tparam SourceReaderType The type of Reader to continuously read from.
   */
  template<typename BufferType, typename SourceReaderType>
  class QueuedReader : private boost::noncopyable {
    public:
      typedef BufferType Buffer;

      //! The source to read from.
      typedef typename TryDereferenceType<SourceReaderType>::type SourceReader;

      //! Constructs a QueuedReader.
      /*!
        \param sourceReader Initializes the source reader.
      */
      template<typename SourceReaderForward>
      QueuedReader(SourceReaderForward&& sourceReader);

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      typename OptionalLocalPtr<SourceReaderType>::type m_sourceReader;
      PipedReader<Buffer> m_queuedReader;
      PipedWriter<Buffer> m_queuedWriter;
      std::atomic_bool m_isReadLoopRunning;
      Routines::RoutineHandler m_readLoopHandler;

      void LaunchReadLoop();
      void ReadLoop();
  };

  template<typename BufferType, typename SourceReaderType>
  template<typename SourceReaderForward>
  QueuedReader<BufferType, SourceReaderType>::QueuedReader(
      SourceReaderForward&& sourceReader)
      : m_sourceReader(std::forward<SourceReaderForward>(sourceReader)),
        m_queuedWriter(Ref(m_queuedReader)),
        m_isReadLoopRunning(false) {}

  template<typename BufferType, typename SourceReaderType>
  bool QueuedReader<BufferType, SourceReaderType>::IsDataAvailable() const {
    LaunchReadLoop();
    return m_queuedReader.IsDataAvailable();
  }

  template<typename BufferType, typename SourceReaderType>
  std::size_t QueuedReader<BufferType, SourceReaderType>::Read(
      Out<BufferType> destination) {
    LaunchReadLoop();
    return m_queuedReader.Read(Store(destination));
  }

  template<typename BufferType, typename SourceReaderType>
  std::size_t QueuedReader<BufferType, SourceReaderType>::Read(
      char* destination, std::size_t size) {
    LaunchReadLoop();
    return m_queuedReader.Read(destination, size);
  }

  template<typename BufferType, typename SourceReaderType>
  std::size_t QueuedReader<BufferType, SourceReaderType>::Read(
      Out<BufferType> destination, std::size_t size) {
    LaunchReadLoop();
    return m_queuedReader.Read(Store(destination), size);
  }

  template<typename BufferType, typename SourceReaderType>
  void QueuedReader<BufferType, SourceReaderType>::LaunchReadLoop() {
    bool previousValue = m_isReadLoopRunning.exchange(true);
    if(!previousValue) {
      m_readLoopHandler = Routines::Spawn(
        std::bind(&QueuedReader::ReadLoop, this));
    }
  }

  template<typename BufferType, typename SourceReaderType>
  void QueuedReader<BufferType, SourceReaderType>::ReadLoop() {
    typename SourceReader::Buffer buffer;
    try {
      while(true) {
        m_sourceReader->Read(Store(buffer));
        m_queuedWriter.Write(buffer);
        buffer.Reset();
      }
    } catch(const std::exception&) {
      m_queuedWriter.Break(std::current_exception());
    }
  }
}

  template<typename BufferType, typename SourceReaderType>
  struct ImplementsConcept<IO::QueuedReader<BufferType, SourceReaderType>,
    IO::Reader<typename IO::QueuedReader<BufferType,
    SourceReaderType>::Buffer>> : std::true_type {};
}

#endif
