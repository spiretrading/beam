#ifndef BEAM_CACHED_VALUE_HPP
#define BEAM_CACHED_VALUE_HPP
#include <functional>
#include <type_traits>

namespace Beam {

  /**
   * Caches the result of a computation.
   * @tparam T The type of the cached value.
   */
  template<typename T>
  class CachedValue {
    public:

      /** The type of the cached value. */
      using Type = T;

      /** The type of function used to compute the value. */
      using Computation = std::function<Type ()>;

      /** Constructs a CachedValue. */
      CachedValue() noexcept(std::is_nothrow_default_constructible_v<Type>);

      /**
       * Constructs a CachedValue.
       * @param computation The computation to cache.
       */
      CachedValue(Computation computation);

      /** Returns a reference to the value. */
      T& operator *() const;

      /** Returns a pointer to the value. */
      T* operator ->() const;

      /**
       * Sets the computation to cache.
       * @param computer The computation to cache.
       */
      void set_computation(const Computation& computation);

      /** Returns the value. */
      T* get() const;

      /** Invalidates this cache. */
      void invalidate();

    private:
      mutable Type m_value;
      mutable bool m_is_valid;
      Computation m_computation;
  };

  template<typename F>
  CachedValue(F) -> CachedValue<std::remove_cvref_t<std::invoke_result_t<F>>>;

  template<typename T>
  CachedValue<T>::CachedValue() noexcept(
    std::is_nothrow_default_constructible_v<Type>)
    : m_is_valid(false) {}

  template<typename T>
  CachedValue<T>::CachedValue(Computation computation)
    : m_is_valid(false),
      m_computation(std::move(computation)) {}

  template<typename T>
  typename CachedValue<T>::Type& CachedValue<T>::operator *() const {
    if(!m_is_valid) {
      m_value = m_computation();
      m_is_valid = true;
    }
    return m_value;
  }

  template<typename T>
  typename CachedValue<T>::Type* CachedValue<T>::operator ->() const {
    if(!m_is_valid) {
      m_value = m_computation();
      m_is_valid = true;
    }
    return &m_value;
  }

  template<typename T>
  void CachedValue<T>::set_computation(const Computation& computation) {
    m_is_valid = false;
    m_computation = computation;
  }

  template<typename T>
  typename CachedValue<T>::Type* CachedValue<T>::get() const {
    if(!m_is_valid) {
      m_value = m_computation();
      m_is_valid = true;
    }
    return &m_value;
  }

  template<typename T>
  void CachedValue<T>::invalidate() {
    m_is_valid = false;
  }
}

#endif
