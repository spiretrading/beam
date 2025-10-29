#ifndef BEAM_REF_HPP
#define BEAM_REF_HPP
#include <concepts>

namespace Beam {

  /**
   * Used to explicitly pass a reference.
   * @tparam T The type being referenced.
   */
  template<typename T>
  class Ref {
    public:

      /** The type being referenced. */
      using Type = T;

      /**
       * Constructs a Ref.
       * @param reference The reference to wrap.
       */
      explicit Ref(Type& reference) noexcept;

      /** Allows for polymorphic Refs. */
      template<typename U>
      Ref(const Ref<U>& reference) noexcept requires
        std::convertible_to<U*, Type*>
        : m_reference(reference.get()) {}

      Ref(const Ref&) = default;
      Ref(Ref&& reference) noexcept;

      /** Returns a pointer to the result. */
      Type* get() const;

      /** Returns a reference to the result. */
      Type& operator *() const;

      /** Returns a pointer to the result. */
      Type* operator ->() const;

      Ref& operator =(const Ref&) = default;
      Ref& operator =(Ref&& reference) noexcept;

    private:
      Type* m_reference;
  };

  template<typename T>
  Ref<T>::Ref(Type& reference) noexcept
    : m_reference(&reference) {}

  template<typename T>
  Ref<T>::Ref(Ref&& ref) noexcept
      : m_reference(ref.m_reference) {
    ref.m_reference = nullptr;
  }

  template<typename T>
  typename Ref<T>::Type* Ref<T>::get() const {
    return m_reference;
  }

  template<typename T>
  typename Ref<T>::Type& Ref<T>::operator *() const {
    return *m_reference;
  }

  template<typename T>
  typename Ref<T>::Type* Ref<T>::operator ->() const {
    return m_reference;
  }

  template<typename T>
  Ref<T>& Ref<T>::operator =(Ref&& reference) noexcept {
    if(this == &reference) {
      return *this;
    }
    m_reference = reference.m_reference;
    reference.m_reference = nullptr;
    return *this;
  }
}

#endif
