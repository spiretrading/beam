#ifndef BEAM_LOCAL_PTR_HPP
#define BEAM_LOCAL_PTR_HPP
#include <boost/mpl/if.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Initializer.hpp"
#include "Beam/Pointers/NativePtr.hpp"
#include "Beam/Pointers/Pointers.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"

namespace Beam {

  /** Wraps a local value as if it were a pointer. */
  template<typename T>
  class LocalPtr {
    public:

      /**
       * Constructs a LocalPtr.
       * @param args The parameters used to initialize the value.
       */
      template<typename... Args>
      LocalPtr(Args&&... args);

      /**
       * Constructs a LocalPtr.
       * @param initializer Stores the parameters used to initialize the value.
       */
      template<typename... Args>
      LocalPtr(Initializer<Args...>&& initializer);

      /**
       * Acquires a LocalPtr.
       * @param ptr The LocalPtr to acquire.
       */
      LocalPtr(LocalPtr&& ptr);

      /** Tests if this LocalPtr is initialized. */
      explicit operator bool() const;

      /** Returns a reference to the value. */
      T& operator *() const;

      /** Returns a pointer to the value. */
      T* operator ->() const;

      /**
       * Copies a LocalPtr.
       * @param ptr The LocalPtr to copy.
       */
      LocalPtr& operator =(const LocalPtr& ptr);

      /**
       * Acquires a LocalPtr.
       * \param ptr The LocalPtr to acquire.
       */
      LocalPtr& operator =(LocalPtr&& ptr);

      /** Returns the value. */
      T& Get() const;

    private:
      struct Wrapper {
        T m_ptr;

        template<typename... Args>
        Wrapper(Args&&... args);

        Wrapper(Wrapper&& wrapper);

        template<typename Tuple, int... Sequence>
        Wrapper(IntegerSequence<Sequence...> sequence, Tuple&& args);
      };
      mutable Wrapper m_wrapper;
  };

  /**
   * Evaluates to a LocalPtr if T is not dereferenceable. Useful when a class
   * must always work using 'pointer' semantics but is using a value behind the
   * scenes.
   */
  template<typename T>
  struct OptionalLocalPtr {
    using type = typename boost::mpl::if_c<IsDereferenceable<T>::value,
      GetOptionalNativePtr<T>, LocalPtr<T>>::type;
  };

  template<typename T>
  using GetOptionalLocalPtr = typename OptionalLocalPtr<T>::type;

  template<typename T>
  template<typename... Args>
  LocalPtr<T>::Wrapper::Wrapper(Args&&... args)
    : m_ptr(std::forward<Args>(args)...) {}

  template<typename T>
  LocalPtr<T>::Wrapper::Wrapper(Wrapper&& wrapper)
    : m_ptr(std::move(wrapper.m_ptr)) {}

  template<typename T>
  template<typename Tuple, int... Sequence>
  LocalPtr<T>::Wrapper::Wrapper(IntegerSequence<Sequence...> sequence,
    Tuple&& args)
    : m_ptr(std::get<Sequence>(std::move(args))...) {}

  template<typename T>
  template<typename... Args>
  LocalPtr<T>::LocalPtr(Args&&... args)
    : m_wrapper(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename... Args>
  LocalPtr<T>::LocalPtr(Initializer<Args...>&& args)
    : m_wrapper(typename IntegerSequenceGenerator<sizeof...(Args)>::type(),
        std::move(args.m_args)) {}

  template<typename T>
  LocalPtr<T>::LocalPtr(LocalPtr&& ptr)
    : m_wrapper(std::move(ptr.m_wrapper)) {}

  template<typename T>
  LocalPtr<T>::operator bool() const {
    return true;
  }

  template<typename T>
  T& LocalPtr<T>::operator *() const {
    return m_wrapper.m_ptr;
  }

  template<typename T>
  T* LocalPtr<T>::operator ->() const {
    return &m_wrapper.m_ptr;
  }

  template<typename T>
  LocalPtr<T>& LocalPtr<T>::operator =(const LocalPtr& ptr) {
    m_wrapper.m_ptr = ptr.m_wrapper.m_ptr;
    return *this;
  }

  template<typename T>
  LocalPtr<T>& LocalPtr<T>::operator =(LocalPtr&& ptr) {
    m_wrapper.m_ptr = std::move(ptr.m_wrapper.m_ptr);
    return *this;
  }

  template<typename T>
  T& LocalPtr<T>::Get() const {
    return m_wrapper.m_ptr;
  }
}

#endif
