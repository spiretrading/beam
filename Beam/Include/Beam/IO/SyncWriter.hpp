#ifndef BEAM_SYNC_WRITER_HPP
#define BEAM_SYNC_WRITER_HPP
#include <mutex>
#include <type_traits>
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /**
   * Synchronizes writes to a destination, allowing it to be shared.
   * @tparam W The Writer to write to.
   */
  template<typename W> requires IsWriter<dereference_t<W>>
  class SyncWriter {
    public:

      /** The destination to write to. */
      using DestinationWriter = dereference_t<W>;

      /**
       * Constructs a SyncWriter.
       * @param destination Used to initialize the destination of all writes.
       */
      template<Initializes<W> WF>
      explicit SyncWriter(WF&& destination);

      template<IsConstBuffer T>
      void write(const T& data);

    private:
      mutable Mutex m_mutex;
      local_ptr_t<W> m_destination;
  };

  template<typename W>
  SyncWriter(W&&) -> SyncWriter<std::remove_cvref_t<W>>;

  template<typename W> requires IsWriter<dereference_t<W>>
  template<Initializes<W> WF>
  SyncWriter<W>::SyncWriter(WF&& destination)
    : m_destination(std::forward<WF>(destination)) {}

  template<typename W> requires IsWriter<dereference_t<W>>
  template<IsConstBuffer B>
  void SyncWriter<W>::write(const B& data) {
    auto lock = std::lock_guard(m_mutex);
    m_destination->write(data);
  }
}

#endif
