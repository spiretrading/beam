#ifndef BEAM_DELAYPTR_HPP
#define BEAM_DELAYPTR_HPP
#include <cassert>
#include <new>
#include <utility>
#include "Beam/Pointers/Initializer.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {

  /*! \class DelayPtr
      \brief Delays the allocation/initialization of a value.
   */
  template<typename T>
  class DelayPtr {
    public:

      //! Constructs a DelayPtr.
      DelayPtr();

      //! Copies a DelayPtr.
      /*!
        \param ptr The DelayPtr to copy.
      */
      DelayPtr(const DelayPtr& ptr);

      //! Acquires a DelayPtr.
      /*!
        \param ptr The DelayPtr to acquire.
      */
      DelayPtr(DelayPtr&& ptr);

      //! Constructs a DelayPtr with an initialized value.
      /*!
        \param args The parameters to initialize the stored value with.
      */
      template<typename Arg, typename... Args>
      DelayPtr(Arg&& arg, Args&&... args);

      //! Constructs a DelayPtr with an initialized value.
      /*!
        \param initializer Stores the parameters used to initialize the stored
               value.
      */
      template<typename... Args>
      DelayPtr(Initializer<Args...>&& initializer);

      ~DelayPtr();

      //! Copies a DelayPtr.
      /*!
        \param ptr The DelayPtr to copy.
      */
      DelayPtr& operator =(const DelayPtr& ptr);

      //! Acquires a DelayPtr.
      /*!
        \param ptr The DelayPtr to acquire.
      */
      DelayPtr& operator =(DelayPtr&& ptr);

      //! Tests if this DelayPtr is initialized.
      operator bool() const;

      //! Returns a reference to the value.
      T& operator *() const;

      //! Returns a pointer to the value.
      T* operator ->() const;

      //! Returns the value.
      T& Get() const;

      //! Returns <code>true</code> iff this is initialized.
      bool IsInitialized() const;

      //! Initializes the stored value.
      /*!
        \param args The parameters to initialize the stored value with.
      */
      template<typename... Args>
      void Initialize(Args&&... args);

      //! Initializes the stored value.
      /*!
        \param initializer Stores the parameters used to initialize the stored
               value.
      */
      template<typename... Args>
      void FromInitializer(Initializer<Args...>&& initializer);

      //! Resets the value.
      void Reset();

    private:
      typedef typename std::aligned_storage<sizeof(T),
        std::alignment_of<T>::value>::type Storage;
      Storage m_storage;
      T* m_ptr;
  };

  /*! \class DelayPtr<void>
      \brief Specialization for working with void Delays.
   */
  template<>
  class DelayPtr<void> {
    public:

      //! Constructs a DelayPtr.
      DelayPtr();

      //! Returns <code>true</code> iff this is initialized.
      bool IsInitialized() const;

      //! Initializes a DelayPtr.
      void Initialize();

      //! Resets the value.
      void Reset();

    private:
      bool m_isInitialized;
  };

  template<typename T>
  DelayPtr<T>::DelayPtr()
      : m_ptr(nullptr) {}

  template<typename T>
  DelayPtr<T>::DelayPtr(const DelayPtr& ptr)
      : m_ptr(nullptr) {
    if(ptr.m_ptr == nullptr) {
      m_ptr = nullptr;
      return;
    }
    Initialize(*ptr);
  }

  template<typename T>
  DelayPtr<T>::DelayPtr(DelayPtr&& ptr)
      : m_ptr(ptr.m_ptr == nullptr ? nullptr :
          new(&m_storage) T(std::move(*ptr.m_ptr))) {
    ptr.Reset();
  }

  template<typename T>
  template<typename Arg, typename... Args>
  DelayPtr<T>::DelayPtr(Arg&& arg, Args&&... args)
      : m_ptr(nullptr) {
    Initialize(std::forward<Arg>(arg), std::forward<Args>(args)...);
  }

  template<typename T>
  template<typename... Args>
  DelayPtr<T>::DelayPtr(Initializer<Args...>&& initializer)
      : m_ptr(nullptr) {
    Initialize(std::move(initializer));
  }

  template<typename T>
  DelayPtr<T>::~DelayPtr() {
    Reset();
  }

  template<typename T>
  DelayPtr<T>& DelayPtr<T>::operator =(const DelayPtr& ptr) {
    if(this == &ptr) {
      return *this;
    }
    Reset();
    if(ptr.m_ptr == nullptr) {
      return *this;
    }
    Initialize(*ptr);
    return *this;
  }

  template<typename T>
  DelayPtr<T>& DelayPtr<T>::operator =(DelayPtr&& ptr) {
    if(this == &ptr) {
      return *this;
    }
    Reset();
    if(ptr.m_ptr == nullptr) {
      return *this;
    }
    m_ptr = new(&m_storage) T(std::move(*ptr.m_ptr));
    ptr.Reset();
    return *this;
  }

  template<typename T>
  DelayPtr<T>::operator bool() const {
    return m_ptr != nullptr;
  }

  template<typename T>
  T& DelayPtr<T>::operator *() const {
    assert(m_ptr != nullptr);
    return *m_ptr;
  }

  template<typename T>
  T* DelayPtr<T>::operator ->() const {
    assert(m_ptr != nullptr);
    return m_ptr;
  }

  template<typename T>
  T& DelayPtr<T>::Get() const {
    assert(m_ptr != nullptr);
    return *m_ptr;
  }

  template<typename T>
  bool DelayPtr<T>::IsInitialized() const {
    return m_ptr != nullptr;
  }
  template<typename T>
  template<typename... Args>
  void DelayPtr<T>::Initialize(Args&&... args) {
    Reset();
    m_ptr = new(&m_storage) T(std::forward<Args>(args)...);
  }

  template<typename T>
  template<typename... Args>
  void DelayPtr<T>::FromInitializer(Initializer<Args...>&& initializer) {
    Reset();
    Apply(std::move(initializer.m_args),
      [&] (Args&&... args) {
        Initialize(static_cast<Args&&>(args)...);
      });
  }

  template<typename T>
  void DelayPtr<T>::Reset() {
    if(m_ptr == nullptr) {
      return;
    }
    m_ptr->~T();
    m_ptr = nullptr;
  }

  inline DelayPtr<void>::DelayPtr()
      : m_isInitialized(false) {}

  inline bool DelayPtr<void>::IsInitialized() const {
    return m_isInitialized;
  }

  inline void DelayPtr<void>::Initialize() {
    m_isInitialized = true;
  }

  inline void DelayPtr<void>::Reset() {
    m_isInitialized = false;
  }
}

#endif
