#ifndef BEAM_TO_PYTHON_READER_HPP
#define BEAM_TO_PYTHON_READER_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Reader.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::Python {

  /**
   * Wraps a Reader for use with Python.
   * @tparam R The type of Reader to wrap.
   */
  template<IsReader R>
  class ToPythonReader {
    public:

      /** The type of Reader to wrap. */
      using Reader = R;

      /**
       * Constructs a ToPythonReader in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonReader(Args&&... args);

      ~ToPythonReader();

      /** Returns a reference to the underlying reader. */
      Reader& get();

      /** Returns a reference to the underlying reader. */
      const Reader& get() const;

      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size = -1);

    private:
      boost::optional<Reader> m_reader;

      ToPythonReader(const ToPythonReader&) = delete;
      ToPythonReader& operator =(const ToPythonReader&) = delete;
  };

  template<typename Reader>
  ToPythonReader(Reader&&) -> ToPythonReader<std::remove_cvref_t<Reader>>;

  template<IsReader R>
  template<typename... Args>
  ToPythonReader<R>::ToPythonReader(Args&&... args)
    : m_reader((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsReader R>
  ToPythonReader<R>::~ToPythonReader() {
    auto release = GilRelease();
    m_reader.reset();
  }

  template<IsReader R>
  typename ToPythonReader<R>::Reader& ToPythonReader<R>::get() {
    return *m_reader;
  }

  template<IsReader R>
  const typename ToPythonReader<R>::Reader& ToPythonReader<R>::get() const {
    return *m_reader;
  }

  template<IsReader R>
  bool ToPythonReader<R>::poll() const {
    auto release = GilRelease();
    return m_reader->poll();
  }

  template<IsReader R>
  template<IsBuffer B>
  std::size_t ToPythonReader<R>::read(Out<B> destination, std::size_t size) {
    auto release = GilRelease();
    return m_reader->read(out(destination), size);
  }
}

#endif
