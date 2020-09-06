#ifndef BEAM_TO_PYTHON_READER_HPP
#define BEAM_TO_PYTHON_READER_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/IO/VirtualReader.hpp"

namespace Beam::IO {

  /**
   * Wraps a Reader for use with Python.
   * @param <R> The type of Reader to wrap.
   */
  template<typename R>
  class ToPythonReader final : public VirtualReader {
    public:

      /** The type of Reader to wrap. */
      using Reader = R;

      /**
       * Constructs a ToPythonReader.
       * @param reader The Reader to wrap.
       */
      ToPythonReader(std::unique_ptr<Reader> reader);

      ~ToPythonReader() override;

      bool IsDataAvailable() const override;

      std::size_t Read(Out<SharedBuffer> destination) override;

      std::size_t Read(char* destination, std::size_t size) override;

      std::size_t Read(Out<SharedBuffer> destination,
        std::size_t size) override;

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
  ToPythonReader<R>::~ToPythonReader() {
    auto release = Python::GilRelease();
    m_reader.reset();
  }

  template<typename R>
  bool ToPythonReader<R>::IsDataAvailable() const {
    auto release = Python::GilRelease();
    return m_reader->IsDataAvailable();
  }

  template<typename R>
  std::size_t ToPythonReader<R>::Read(Out<SharedBuffer> destination) {
    auto release = Python::GilRelease();
    return m_reader->Read(Store(destination));
  }

  template<typename R>
  std::size_t ToPythonReader<R>::Read(char* destination, std::size_t size) {
    auto release = Python::GilRelease();
    return m_reader->Read(destination, size);
  }

  template<typename R>
  std::size_t ToPythonReader<R>::Read(Out<SharedBuffer> destination,
      std::size_t size) {
    auto release = Python::GilRelease();
    return m_reader->Read(Store(destination), size);
  }
}

#endif
