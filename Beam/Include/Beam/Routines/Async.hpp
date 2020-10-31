#ifndef BEAM_ASYNC_HPP
#define BEAM_ASYNC_HPP
#include <type_traits>
#include <boost/call_traits.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
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
      mutable boost::mutex m_mutex;
      State m_state;
      SuspendedRoutineQueue m_suspendedRoutines;
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
}

#endif
