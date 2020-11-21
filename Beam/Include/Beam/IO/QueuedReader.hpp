#ifndef BEAM_QUEUED_READER_HPP
#define BEAM_QUEUED_READER_HPP
#include <atomic>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {
namespace IO {

  /**
   * Continuously reads data into a queue, asynchronously.
   * @param <B> The type of Buffer to read into.
   * @param <R> The type of Reader to continuously read from.
   */
  template<typename B, typename R>
  class QueuedReader {
    public:
      using Buffer = B;

      /** The source to read from. */
      using SourceReader = GetTryDereferenceType<R>;

      /**
       * Constructs a QueuedReader.
       * @param sourceReader Initializes the source reader.
       */
      template<typename RF>
      QueuedReader(RF&& sourceReader);

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      GetOptionalLocalPtr<R> m_sourceReader;
      PipedReader<Buffer> m_queuedReader;
      PipedWriter<Buffer> m_queuedWriter;
      std::atomic_bool m_isReadLoopRunning;
      Routines::RoutineHandler m_readLoopHandler;

      void LaunchReadLoop();
      void ReadLoop();
  };

  template<typename B, typename R>
  template<typename RF>
  QueuedReader<B, R>::QueuedReader(RF&& sourceReader)
    : m_sourceReader(std::forward<RF>(sourceReader)),
      m_queuedWriter(Ref(m_queuedReader)),
      m_isReadLoopRunning(false) {}

  template<typename B, typename R>
  bool QueuedReader<B, R>::IsDataAvailable() const {
    LaunchReadLoop();
    return m_queuedReader.IsDataAvailable();
  }

  template<typename B, typename R>
  std::size_t QueuedReader<B, R>::Read(Out<B> destination) {
    LaunchReadLoop();
    return m_queuedReader.Read(Store(destination));
  }

  template<typename B, typename R>
  std::size_t QueuedReader<B, R>::Read(char* destination, std::size_t size) {
    LaunchReadLoop();
    return m_queuedReader.Read(destination, size);
  }

  template<typename B, typename R>
  std::size_t QueuedReader<B, R>::Read(Out<B> destination, std::size_t size) {
    LaunchReadLoop();
    return m_queuedReader.Read(Store(destination), size);
  }

  template<typename B, typename R>
  void QueuedReader<B, R>::LaunchReadLoop() {
    if(!m_isReadLoopRunning.exchange(true)) {
      m_readLoopHandler = Routines::Spawn(
        std::bind(&QueuedReader::ReadLoop, this));
    }
  }

  template<typename B, typename R>
  void QueuedReader<B, R>::ReadLoop() {
    auto buffer = typename SourceReader::Buffer();
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

  template<typename B, typename R>
  struct ImplementsConcept<IO::QueuedReader<B, R>,
    IO::Reader<typename IO::QueuedReader<B, R>::Buffer>> : std::true_type {};
}

#endif
