#ifndef BEAM_TO_PYTHON_WRITER_HPP
#define BEAM_TO_PYTHON_WRITER_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/IO/VirtualWriter.hpp"

namespace Beam::IO {

  /**
   * Wraps a Writer for use with Python.
   * @param <W> The type of Writer to wrap.
   */
  template<typename W>
  class ToPythonWriter final : public VirtualWriter {
    public:

      /** The type of Writer to wrap. */
      using Writer = W;

      /**
       * Constructs a ToPythonWriter.
       * @param writer The Writer to wrap.
       */
      ToPythonWriter(std::unique_ptr<Writer> writer);

      ~ToPythonWriter() override;

      void Write(const void* data, std::size_t size) override;

      void Write(const Buffer& data) override;

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
  void ToPythonWriter<W>::Write(const Buffer& data) {
    auto release = Python::GilRelease();
    m_writer->Write(data);
  }
}

#endif
