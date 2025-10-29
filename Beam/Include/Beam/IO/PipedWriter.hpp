#ifndef BEAM_PIPED_WRITER_HPP
#define BEAM_PIPED_WRITER_HPP
#include <memory>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {
  class PipedReader;

  /** Writes to a PipedReader. */
  class PipedWriter {
    public:

      /**
       * Constructs a PipedWriter.
       * @param destination The PipedReader to connect to.
       */
      explicit PipedWriter(Ref<PipedReader> destination);

      ~PipedWriter();

      /**
       * Breaks the pipe.
       * @param e The cause of the break.
       */
      void close(const std::exception_ptr& e);

      /**
       * Breaks the pipe.
       * @param e The cause of the break.
       */
      template<typename E>
      void close(const E& e);

      /** Breaks the pipe. */
      void close();

      template<IsConstBuffer T>
      void write(const T& data);

    private:
      std::shared_ptr<Queue<BufferReader<SharedBuffer>>> m_messages;

      PipedWriter(const PipedWriter&) = delete;
      PipedWriter& operator =(const PipedWriter&) = delete;
  };
}

#endif

#include "Beam/IO/PipedReader.hpp"
