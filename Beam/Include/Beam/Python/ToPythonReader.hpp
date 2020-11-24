#ifndef BEAM_TO_PYTHON_READER_HPP
#define BEAM_TO_PYTHON_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::IO {

  /**
   * Wraps a Reader for use with Python.
   * @param <R> The type of Reader to wrap.
   */
  template<typename R>
  class ToPythonReader {
    public:

      /** The type of Reader to wrap. */
      using Reader = R;

      /**
       * Constructs a ToPythonReader.
       * @param reader The Reader to wrap.
       */
      ToPythonReader(std::unique_ptr<Reader> reader);

      /**
       * Constructs a ToPythonReader in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonReader(Args&&... args);

      ~ToPythonReader();

      /** Returns the wrapped Reader. */
      const Reader& GetReader() const;

      /** Returns the wrapped Reader. */
      Reader& GetReader();

      bool IsDataAvailable() const;

      std::size_t Read(Out<BufferBox> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<BufferBox> destination, std::size_t size);

    private:
      std::unique_ptr<Reader> m_reader;
  };

  /**
   * Makes a ToPythonReader.
   * @param reader The Reader to wrap.
   */
  template<typename Reader>
  auto MakeToPythonReader(std::unique_ptr<Reader> reader) {
    return std::make_unique<ToPythonReader<Reader>>(std::move(reader));
  }

  template<typename R>
  ToPythonReader<R>::ToPythonReader(std::unique_ptr<Reader> reader)
    : m_reader(std::move(reader)) {}

  template<typename R>
  template<typename... Args>
  ToPythonReader<R>::ToPythonReader(Args&&... args)
    : ToPythonReader(std::make_unique<Reader>(std::forward<Args>(args)...)) {}

  template<typename R>
  ToPythonReader<R>::~ToPythonReader() {
    auto release = Python::GilRelease();
    m_reader.reset();
  }

  template<typename R>
  const typename ToPythonReader<R>::Reader&
      ToPythonReader<R>::GetReader() const {
    return *m_reader;
  }

  template<typename R>
  typename ToPythonReader<R>::Reader& ToPythonReader<R>::GetReader() {
    return *m_reader;
  }

  template<typename R>
  bool ToPythonReader<R>::IsDataAvailable() const {
    auto release = Python::GilRelease();
    return m_reader->IsDataAvailable();
  }

  template<typename R>
  std::size_t ToPythonReader<R>::Read(Out<BufferBox> destination) {
    auto release = Python::GilRelease();
    return m_reader->Read(Store(destination));
  }

  template<typename R>
  std::size_t ToPythonReader<R>::Read(char* destination, std::size_t size) {
    auto release = Python::GilRelease();
    return m_reader->Read(destination, size);
  }

  template<typename R>
  std::size_t ToPythonReader<R>::Read(Out<BufferBox> destination,
      std::size_t size) {
    auto release = Python::GilRelease();
    return m_reader->Read(Store(destination), size);
  }
}

#endif
