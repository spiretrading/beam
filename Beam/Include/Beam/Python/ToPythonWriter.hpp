#ifndef BEAM_TO_PYTHON_WRITER_HPP
#define BEAM_TO_PYTHON_WRITER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
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
       * Constructs a ToPythonWriter in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonWriter(Args&&... args);

      ~ToPythonWriter();

      ToPythonWriter(ToPythonWriter&&) = default;

      void Write(const void* data, std::size_t size);

      void Write(const BufferView& data);

      ToPythonWriter& operator =(ToPythonWriter&&) = default;

    private:
      boost::optional<Writer> m_writer;

      ToPythonWriter(const ToPythonWriter&) = delete;
      ToPythonWriter& operator =(const ToPythonWriter&) = delete;
  };

  template<typename Writer>
  ToPythonWriter(Writer&&) -> ToPythonWriter<std::decay_t<Writer>>;

  template<typename R>
  template<typename... Args>
  ToPythonWriter<R>::ToPythonWriter(Args&&... args)
    : m_writer((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

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
