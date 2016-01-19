#ifndef BEAM_CAPTURE_HPP
#define BEAM_CAPTURE_HPP
#include <utility>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Capture
      \brief Used to capture an r-value reference into a lambda expression.
   */
  template<typename T>
  class Capture {
    public:

      //! Constructs a Capture.
      /*!
        \param value The value to capture.
      */
      Capture(T&& value);

      //! Copies a Capture.
      /*!
        \param capture The Capture to copy.
      */
      Capture(const Capture& capture);

      //! Returns a reference to the value.
      T& operator *() const;

      //! Returns a pointer to the value.
      T* operator ->() const;

      //! Returns the value.
      T* Get() const;

    private:
      mutable T m_value;
  };

  //! Captures a value.
  /*!
    \param value The value to Capture.
    \return A Capture of the <i>value</i>.
  */
  template<typename T>
  Capture<T> MakeCapture(T&& value) {
    return Capture<T>(std::move(value));
  }

  template<typename T>
  Capture<T>::Capture(T&& value)
      : m_value(std::move(value)) {}

  template<typename T>
  Capture<T>::Capture(const Capture& capture)
      : m_value(std::move(capture.m_value)) {}

  template<typename T>
  T& Capture<T>::operator *() const {
    return m_value;
  }

  template<typename T>
  T* Capture<T>::operator ->() const {
    return &m_value;
  }

  template<typename T>
  T* Capture<T>::Get() const {
    return &m_value;
  }
}

#endif
