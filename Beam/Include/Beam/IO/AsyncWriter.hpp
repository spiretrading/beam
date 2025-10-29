#ifndef BEAM_ASYNC_WRITER_HPP
#define BEAM_ASYNC_WRITER_HPP
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"

namespace Beam {

  /**
   * Asynchronously writes to a destination using a Routine.
   * @tparam W The Writer to write to.
   */
  template<typename W> requires IsWriter<dereference_t<W>>
  class AsyncWriter {
    public:

      /** The destination to write to. */
      using DestinationWriter = dereference_t<W>;

      /**
       * Constructs an AsyncWriter.
       * @param destination Used to initialize the destination of all writes.
       */
      template<Initializes<W> WF>
      explicit AsyncWriter(WF&& destination);

      template<IsConstBuffer T>
      void write(const T& data);
      template<IsConstBuffer T>
      void write(T&& data);

    private:
      local_ptr_t<W> m_destination;
      std::exception_ptr m_exception;
      RoutineTaskQueue m_tasks;
  };

  template<typename W>
  AsyncWriter(W&&) -> AsyncWriter<std::remove_cvref_t<W>>;

  template<typename W> requires IsWriter<dereference_t<W>>
  template<Initializes<W> WF>
  AsyncWriter<W>::AsyncWriter(WF&& destination)
    : m_destination(std::forward<WF>(destination)) {}

  template<typename W> requires IsWriter<dereference_t<W>>
  template<IsConstBuffer B>
  void AsyncWriter<W>::write(const B& data) {
    try {
      m_tasks.push([=, this] {
        try {
          m_destination->write(data);
        } catch(const std::exception&) {
          if(!m_exception) {
            m_exception = std::current_exception();
            m_tasks.close();
            boost::throw_with_location(PipeBrokenException());
          }
        }
      });
    } catch(const PipeBrokenException&) {
      std::rethrow_exception(m_exception);
    }
  }

  template<typename W> requires IsWriter<dereference_t<W>>
  template<IsConstBuffer B>
  void AsyncWriter<W>::write(B&& data) {
    try {
      m_tasks.push([=, data = std::move(data), this] {
        try {
          m_destination->write(data);
        } catch(const std::exception&) {
          if(!m_exception) {
            m_exception = std::current_exception();
            m_tasks.close();
            boost::throw_with_location(PipeBrokenException());
          }
        }
      });
    } catch(const PipeBrokenException&) {
      std::rethrow_exception(m_exception);
    }
  }
}

#endif
