#ifndef BEAM_TO_PYTHON_WRITER_HPP
#define BEAM_TO_PYTHON_WRITER_HPP
#include "Beam/IO/Writer.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::IO {

  /**
   * Wraps a Writer for use with Python.
   * @param <W> The type of Writer to wrap.
   */
  template<typename W>
  class ToPythonWriter {
    public:

      /** The type of Writer to wrap. */
      using Writer = W;

      /**
       * Constructs a ToPythonWriter.
       * @param writer The Writer to wrap.
       */
      ToPythonWriter(std::unique_ptr<Writer> writer);

      ~ToPythonWriter();

      void Write(const void* data, std::size_t size);

      void Write(const BufferView& data);

    private:
      std::unique_ptr<Writer> m_writer;
  };

  /**
   * Makes a ToPythonWriter.
   * @param writer The Writer to wrap.
   */
  template<typename Writer>
  auto MakeToPythonWriter(std::unique_ptr<Writer> writer) {
    return std::make_unique<ToPythonWriter<Writer>>(std::move(writer));
  }

  template<typename W>
  ToPythonWriter<W>::ToPythonWriter(std::unique_ptr<Writer> writer)
    : m_writer(std::move(writer)) {}

  template<typename W>
  ToPythonWriter<W>::~ToPythonWriter() {
    auto release = Python::GilRelease();
    m_writer.reset();
  }

  template<typename W>
  void ToPythonWriter<W>::Write(const void* data, std::size_t size) {
    auto release = Python::GilRelease();
    m_writer->Write(data, size);
  }

  template<typename W>
  void ToPythonWriter<W>::Write(const BufferView& data) {
    auto release = Python::GilRelease();
    m_writer->Write(data);
  }
}

#endif
