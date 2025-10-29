#ifndef BEAM_TO_PYTHON_WRITER_HPP
#define BEAM_TO_PYTHON_WRITER_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Writer.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::Python {

  /**
   * Wraps a Writer for use with Python.
   * @tparam W The type of Writer to wrap.
   */
  template<IsWriter W>
  class ToPythonWriter {
    public:

      /** The type of Writer to wrap. */
      using Writer = W;

      /**
       * Constructs a ToPythonWriter in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonWriter(Args&&... args);

      ~ToPythonWriter();

      /** Returns a reference to the underlying writer. */
      Writer& get();

      /** Returns a reference to the underlying writer. */
      const Writer& get() const;

      /** Returns a reference to the underlying writer. */
      Writer& operator *();

      /** Returns a reference to the underlying writer. */
      const Writer& operator *() const;

      /** Returns a pointer to the underlying writer. */
      Writer* operator ->();

      /** Returns a pointer to the underlying writer. */
      const Writer* operator ->() const;

      template<IsConstBuffer B>
      void write(const B& data);

    private:
      boost::optional<Writer> m_writer;

      ToPythonWriter(const ToPythonWriter&) = delete;
      ToPythonWriter& operator =(const ToPythonWriter&) = delete;
  };

  template<typename Writer>
  ToPythonWriter(Writer&&) -> ToPythonWriter<std::remove_cvref_t<Writer>>;

  template<IsWriter W>
  template<typename... Args>
  ToPythonWriter<W>::ToPythonWriter(Args&&... args)
    : m_writer((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsWriter W>
  ToPythonWriter<W>::~ToPythonWriter() {
    auto release = GilRelease();
    m_writer.reset();
  }

  template<IsWriter W>
  typename ToPythonWriter<W>::Writer& ToPythonWriter<W>::get() {
    return *m_writer;
  }

  template<IsWriter W>
  const typename ToPythonWriter<W>::Writer& ToPythonWriter<W>::get() const {
    return *m_writer;
  }

  template<IsWriter W>
  typename ToPythonWriter<W>::Writer& ToPythonWriter<W>::operator *() {
    return *m_writer;
  }

  template<IsWriter W>
  const typename ToPythonWriter<W>::Writer& ToPythonWriter<W>::operator *()
      const {
    return *m_writer;
  }

  template<IsWriter W>
  typename ToPythonWriter<W>::Writer* ToPythonWriter<W>::operator ->() {
    return m_writer.get_ptr();
  }

  template<IsWriter W>
  const typename ToPythonWriter<W>::Writer* ToPythonWriter<W>::operator ->()
      const {
    return m_writer.get_ptr();
  }

  template<IsWriter W>
  template<IsConstBuffer B>
  void ToPythonWriter<W>::write(const B& data) {
    auto release = GilRelease();
    m_writer->write(data);
  }
}

#endif
