#ifndef BEAM_TO_PYTHON_READER_HPP
#define BEAM_TO_PYTHON_READER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
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
       * Constructs a ToPythonReader in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonReader(Args&&... args);

      ~ToPythonReader();

      ToPythonReader(ToPythonReader&&) = default;

      /** Returns the wrapped Reader. */
      const Reader& GetReader() const;

      /** Returns the wrapped Reader. */
      Reader& GetReader();

      bool IsDataAvailable() const;

      std::size_t Read(Out<BufferBox> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<BufferBox> destination, std::size_t size);

      ToPythonReader& operator =(ToPythonReader&&) = default;

    private:
      boost::optional<Reader> m_reader;

      ToPythonReader(const ToPythonReader&) = delete;
      ToPythonReader& operator =(const ToPythonReader&) = delete;
  };

  template<typename Reader>
  ToPythonReader(Reader&&) -> ToPythonReader<std::decay_t<Reader>>;

  template<typename R>
  template<typename... Args>
  ToPythonReader<R>::ToPythonReader(Args&&... args)
    : m_reader((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

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
