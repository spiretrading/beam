#ifndef BEAM_ASYNCWRITER_HPP
#define BEAM_ASYNCWRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"

namespace Beam {
namespace IO {

  /*! \class AsyncWriter
      \brief Asynchronously writes to a destination using a Routine.
      \tparam DestinationWriterType The Writer to write to.
   */
  template<typename DestinationWriterType>
  class AsyncWriter : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      //! The destination to write to.
      using DestinationWriter =
        typename TryDereferenceType<DestinationWriterType>::type;

      //! Constructs an AsyncWriter.
      /*!
        \param destination Used to initialize the destination of all writes.
      */
      template<typename DestinationWriterForward>
      AsyncWriter(DestinationWriterForward&& destination);

      void Write(const void* data, std::size_t size);

      void Write(const SharedBuffer& data);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      typename OptionalLocalPtr<DestinationWriterType>::type m_destination;
      RoutineTaskQueue m_tasks;
  };

  template<typename DestinationWriterType>
  template<typename DestinationWriterForward>
  AsyncWriter<DestinationWriterType>::AsyncWriter(
      DestinationWriterForward&& destination)
      : m_destination(std::forward<DestinationWriterForward>(destination)) {}

  template<typename DestinationWriterType>
  void AsyncWriter<DestinationWriterType>::Write(const void* data,
      std::size_t size) {
    SharedBuffer buffer(data, size);
    Write(buffer);
  }

  template<typename DestinationWriterType>
  void AsyncWriter<DestinationWriterType>::Write(const SharedBuffer& data) {
    m_tasks.Push(
      [=] {
        m_destination->Write(data);
      });
  }

  template<typename DestinationWriterType>
  template<typename BufferType>
  void AsyncWriter<DestinationWriterType>::Write(const BufferType& data) {
    SharedBuffer buffer = data;
    Write(buffer);
  }
}

  template<typename BufferType, typename DestinationWriterType>
  struct ImplementsConcept<IO::AsyncWriter<DestinationWriterType>,
    IO::Writer<BufferType>> : std::true_type {};
}

#endif
