#ifndef BEAM_ASYNC_HPP
#define BEAM_ASYNC_HPP
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <utility>
#include <boost/call_traits.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Utilities/StorageType.hpp"

namespace Beam::Routines {

  /** Stores details common to all the Async templates. */
  class BaseAsync {
    public:

      /** Enumerates the states of an operation. */
      enum class State {

        /** The operation is currently pending. */
        PENDING,

        /** The operation succeeded. */
        COMPLETE,

        /** The operation resulted in an exception. */
        EXCEPTION
      };

    protected:

      /** Constructs an empty BaseAsync. */
      BaseAsync() = default;

    private:
      BaseAsync(const BaseAsync&) = delete;
      BaseAsync& operator =(const BaseAsync&) = delete;
  };

  /**
   * Stores the result of an asynchronous operation.
   * @param <T> The type of the asynchronous result.
   */
  template<typename T>
  class Async final : public BaseAsync {
    public:

      /** The type of the asynchronous result. */
      using Type = T;

      /** Constructs an Async. */
      Async();

      /** Returns an object that can be used to evaluate this Async. */
      Eval<Type> GetEval();

      /** Returns the result of the operation. */
      typename boost::call_traits<typename StorageType<Type>::type>::reference
        Get();

      /** Returns the exception. */
      const std::exception_ptr& GetException() const;

      /** Returns the state of this Async. */
      State GetState() const;

      /** Resets this Async so that it can be reused. */
      void Reset();

    private:
      friend class Eval<Type>;
      mutable std::mutex m_mutex;
      State m_state;
      std::condition_variable m_isCompleteCondition;
      boost::optional<typename StorageType<Type>::type> m_result;
      std::exception_ptr m_exception;

      void SetState(State state);
  };

  /** Base class for the Eval template. */
  class BaseEval {
    public:
      virtual ~BaseEval() = default;

      /**
       * Sets an exception to be returned.
       * @param e The exception to set.
       */
      template<typename E>
      void SetException(const E& e);

      /**
       * Sets an exception to be returned.
       * @param e The exception to set.
       */
      virtual void SetException(const std::exception_ptr& e) = 0;

    protected:

      /** Constructs an empty BaseEval. */
      BaseEval() = default;

    private:
      BaseEval(const BaseEval&) = delete;
      BaseEval& operator =(const BaseEval&) = delete;
  };

  /**
   * Evaluates the result of an asynchronous operation.
   * @param <T> The type of the asynchronous operation.
   */
  template<typename T>
  class Eval final : public BaseEval {
    public:

      /** The type of the asynchronous result. */
      using Type = T;

      /** Constructs an Eval whose result is discarded. */
      Eval();

      /**
       * Acquires an Eval.
       * @param eval The Eval to acquire.
       */
      Eval(Eval&& eval);

      /**
       * Acquires an Eval.
       * @param rhs The Eval to acquire.
       */
      Eval& operator =(Eval&& rhs);

      /**
       * Returns <code>true</code> iff no Async is associated with this Eval.
       */
      bool IsEmpty() const;

      /**
       * Sets the result of this Eval.
       * @param result The result of this Eval.
       */
      template<typename R>
      void SetResult(R&& result);

      /** Sets the result of this Eval. */
      void SetResult();

      /** Emplaces the result of this Eval. */
      template<typename... R>
      void Emplace(R&&... result);

      virtual void SetException(const std::exception_ptr& e);

      using BaseEval::SetException;
    private:
      friend class Async<Type>;
      Async<Type>* m_async;

      Eval(Ref<Async<Type>> async);
  };

  template<typename T>
  Async<T>::Async()
    : m_state(State::PENDING) {}

  template<typename T>
  Eval<typename Async<T>::Type> Async<T>::GetEval() {
    Reset();
    return {Ref(*this)};
  }

  template<typename T>
  const std::exception_ptr& Async<T>::GetException() const {
    return m_exception;
  }

  template<typename T>
  typename boost::call_traits<typename StorageType<T>::type>::reference
      Async<T>::Get() {
    auto lock = std::unique_lock(m_mutex);
    while(m_state == State::PENDING) {
      m_isCompleteCondition.wait(lock);
    }
    if(m_state == State::EXCEPTION) {
      std::rethrow_exception(m_exception);
    }
    return VoidReturn(*m_result);
  }

  template<typename T>
  BaseAsync::State Async<T>::GetState() const {
    return m_state;
  }

  template<typename T>
  void Async<T>::Reset() {
    if(m_state == State::PENDING) {
      return;
    }
    m_exception = nullptr;
    m_state = State::PENDING;
    m_result = boost::none;
  }

  template<typename T>
  void Async<T>::SetState(State state) {
    auto lock = std::lock_guard(m_mutex);
    assert(m_state == State::PENDING);
    assert(state != State::PENDING);
    m_state = state;
    m_isCompleteCondition.notify_all();
  }

  template<typename E>
  void BaseEval::SetException(const E& e) {
    SetException(std::make_exception_ptr(e));
  }

  template<typename T>
  Eval<T>::Eval()
    : m_async(nullptr) {}

  template<typename T>
  Eval<T>::Eval(Eval&& eval) {
    *this = std::move(eval);
  }

  template<typename T>
  Eval<T>& Eval<T>::operator =(Eval&& rhs) {
    if(this == &rhs) {
      return *this;
    }
    m_async = rhs.m_async;
    rhs.m_async = nullptr;
    return *this;
  }

  template<typename T>
  bool Eval<T>::IsEmpty() const {
    return m_async == nullptr;
  }

  template<typename T>
  template<typename R>
  void Eval<T>::SetResult(R&& result) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    assert(async->m_state == BaseAsync::State::PENDING);
    async->m_result.emplace(std::forward<R>(result));
    async->SetState(BaseAsync::State::COMPLETE);
  }

  template<typename T>
  void Eval<T>::SetResult() {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    assert(async->m_state == BaseAsync::State::PENDING);
    async->m_result.emplace();
    async->SetState(BaseAsync::State::COMPLETE);
  }

  template<typename T>
  template<typename... R>
  void Eval<T>::Emplace(R&&... result) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    assert(async->m_state == BaseAsync::State::PENDING);
    async->m_result.emplace(std::forward<R>(result)...);
    async->SetState(BaseAsync::State::COMPLETE);
  }

  template<typename T>
  void Eval<T>::SetException(const std::exception_ptr& e) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    assert(async->m_state == BaseAsync::State::PENDING);
    async->m_exception = e;
    async->SetState(BaseAsync::State::EXCEPTION);
  }

  template<typename T>
  Eval<T>::Eval(Ref<Async<Type>> async)
    : m_async(async.Get()) {}
}

#endif
