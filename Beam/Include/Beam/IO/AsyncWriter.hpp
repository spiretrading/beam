#ifndef BEAM_ASYNC_WRITER_HPP
#define BEAM_ASYNC_WRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"

namespace Beam {
namespace IO {

  /**
   * Asynchronously writes to a destination using a Routine.
   * @param <W> The Writer to write to.
   */
  template<typename W>
  class AsyncWriter {
    public:
      using Buffer = SharedBuffer;

      /** The destination to write to. */
      using DestinationWriter = GetTryDereferenceType<W>;

      /**
       * Constructs an AsyncWriter.
       * @param destination Used to initialize the destination of all writes.
       */
      template<typename WF>
      AsyncWriter(WF&& destination);

      void Write(const void* data, std::size_t size);

      void Write(const SharedBuffer& data);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      GetOptionalLocalPtr<W> m_destination;
      std::exception_ptr m_exception;
      RoutineTaskQueue m_tasks;
  };

  template<typename W>
  template<typename WF>
  AsyncWriter<W>::AsyncWriter(WF&& destination)
    : m_destination(std::forward<WF>(destination)) {}

  template<typename W>
  void AsyncWriter<W>::Write(const void* data, std::size_t size) {
    Write(SharedBuffer(data, size));
  }

  template<typename W>
  void AsyncWriter<W>::Write(const SharedBuffer& data) {
    try {
      m_tasks.Push([=] {
        try {
          m_destination->Write(data);
        } catch(const std::exception&) {
          if(!m_exception) {
            m_exception = std::current_exception();
            m_tasks.Break();
          }
        }
      });
    } catch(const PipeBrokenException&) {
      std::rethrow_exception(m_exception);
    }
  }

  template<typename W>
  template<typename BufferType>
  void AsyncWriter<W>::Write(const BufferType& data) {
    Write(data);
  }
}

  template<typename BufferType, typename W>
  struct ImplementsConcept<IO::AsyncWriter<W>, IO::Writer<BufferType>> :
    std::true_type {};
}

#endif
