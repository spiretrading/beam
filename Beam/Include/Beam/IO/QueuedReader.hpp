#ifndef BEAM_QUEUED_READER_HPP
#define BEAM_QUEUED_READER_HPP
#include <atomic>
#include <type_traits>
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

  /**
   * Continuously reads data into a queue, asynchronously.
   * @tparam R The type of Reader to continuously read from.
   */
  template<typename R> requires IsReader<dereference_t<R>>
  class QueuedReader {
    public:

      /** The source to read from. */
      using SourceReader = dereference_t<R>;

      /**
       * Constructs a QueuedReader.
       * @param reader Initializes the source reader.
       */
      template<Initializes<R> RF>
      explicit QueuedReader(RF&& reader);

      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size = -1);

    private:
      local_ptr_t<R> m_reader;
      PipedReader m_queued_reader;
      PipedWriter m_queued_writer;
      std::atomic_bool m_is_reading;
      RoutineHandler m_loop;

      void launch();
      void loop();
  };

  template<typename R>
  QueuedReader(R&&) -> QueuedReader<std::remove_cvref_t<R>>;

  template<typename R> requires IsReader<dereference_t<R>>
  template<Initializes<R> RF>
  QueuedReader<R>::QueuedReader(RF&& reader)
    : m_reader(std::forward<RF>(reader)),
      m_queued_writer(Ref(m_queued_reader)),
      m_is_reading(false) {}

  template<typename R> requires IsReader<dereference_t<R>>
  bool QueuedReader<R>::poll() const {
    const_cast<QueuedReader*>(this)->launch();
    return m_queued_reader.poll();
  }

  template<typename R> requires IsReader<dereference_t<R>>
  template<IsBuffer B>
  std::size_t QueuedReader<R>::read(Out<B> destination, std::size_t size) {
    launch();
    return m_queued_reader.read(out(destination), size);
  }

  template<typename R> requires IsReader<dereference_t<R>>
  void QueuedReader<R>::launch() {
    if(!m_is_reading.exchange(true)) {
      m_loop = spawn(std::bind_front(&QueuedReader::loop, this));
    }
  }

  template<typename R> requires IsReader<dereference_t<R>>
  void QueuedReader<R>::loop() {
    auto buffer = SharedBuffer();
    try {
      while(true) {
        m_reader->read(out(buffer));
        m_queued_writer.write(buffer);
        reset(buffer);
      }
    } catch(const std::exception&) {
      m_queued_writer.close(std::current_exception());
    }
  }
}

#endif
