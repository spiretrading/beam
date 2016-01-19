#ifndef BEAM_CACHEDVALUE_HPP
#define BEAM_CACHEDVALUE_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class CachedValue
      \brief Caches the result of a computation.
   */
  template<typename T>
  class CachedValue : private boost::noncopyable {
    public:

      //! Constructs a CachedValue.
      CachedValue();

      //! Constructs a CachedValue.
      /*!
        \param computation The computation to cache.
      */
      CachedValue(const std::function<T ()>& computation);

      //! Sets the computation to cache.
      /*!
        \param computer The computation to cache.
      */
      void SetComputation(const std::function<T ()>& computation);

      //! Invalidates this cache.
      void Invalidate();

      //! Returns a reference to the value.
      T& operator *() const;

      //! Returns a pointer to the value.
      T* operator ->() const;

      //! Returns the value.
      T* Get() const;

    private:
      mutable T m_value;
      mutable bool m_isValid;
      std::function<T ()> m_computation;
  };

  template<typename T>
  CachedValue<T>::CachedValue()
      : m_isValid(false) {}

  template<typename T>
  CachedValue<T>::CachedValue(const std::function<T ()>& computation)
      : m_isValid(false),
        m_computation(computation) {}

  template<typename T>
  void CachedValue<T>::SetComputation(const std::function<T ()>& computation) {
    m_isValid = false;
    m_computation = computation;
  }

  template<typename T>
  void CachedValue<T>::Invalidate() {
    m_isValid = false;
  }

  template<typename T>
  T& CachedValue<T>::operator *() const {
    if(!m_isValid) {
      m_value = m_computation();
      m_isValid = true;
    }
    return m_value;
  }

  template<typename T>
  T* CachedValue<T>::operator ->() const {
    if(!m_isValid) {
      m_value = m_computation();
      m_isValid = true;
    }
    return &m_value;
  }

  template<typename T>
  T* CachedValue<T>::Get() const {
    if(!m_isValid) {
      m_value = m_computation();
      m_isValid = true;
    }
    return &m_value;
  }
}

#endif
