#ifndef BEAM_ASYNC_HPP
#define BEAM_ASYNC_HPP
#include <cassert>
#include <type_traits>
#include <boost/call_traits.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/StorageType.hpp"

namespace Beam {
namespace Routines {

  /*! \class BaseAsync
      \brief Stores details common to all the Async templates.
   */
  class BaseAsync : private boost::noncopyable {
    public:

      /*! \enum State
          \brief Enumerates the states of an operation.
       */
      enum class State {

        //! The operation is currently pending.
        PENDING,

        //! The operation succeeded.
        COMPLETE,

        //! The operation resulted in an exception.
        EXCEPTION
      };
  };

  /*! \class Async
      \brief Stores the result of an asynchronous operation.
      \tparam T The type of the asynchronous result.
   */
  template<typename T>
  class Async : public BaseAsync {
    public:

      //! Constructs an Async.
      Async();

      //! Returns an object that can be used to evaluate this Async.
      Eval<T> GetEval();

      //! Returns the result of the operation.
      typename boost::call_traits<typename StorageType<T>::type>::reference
        Get();

      //! Returns the exception.
      const std::exception_ptr& GetException() const;

      //! Returns the state of this Async.
      State GetState() const;

      //! Resets this Async so that it can be reused.
      void Reset();

    private:
      friend class Eval<T>;
      mutable boost::mutex m_mutex;
      State m_state;
      Threading::Sync<SuspendedRoutineQueue> m_suspendedRoutines;
      DelayPtr<typename StorageType<T>::type> m_result;
      std::exception_ptr m_exception;

      void SetState(State state);
  };

  /*! \class BaseEval
      \brief Base class for the Eval template.
   */
  class BaseEval : private boost::noncopyable {
    public:
      virtual ~BaseEval() = default;

      //! Sets an exception to be returned.
      /*!
        \param e The exception to set.
      */
      template<typename E>
      void SetException(const E& e);

      //! Sets an exception to be returned.
      /*!
        \param e The exception to set.
      */
      virtual void SetException(const std::exception_ptr& e) = 0;
  };

  /*! \class Eval
      \brief Evaluates the result of an asynchronous operation.
   */
  template<typename T>
  class Eval : public BaseEval {
    public:

      //! Constructs an Eval whose result is discarded.
      Eval();

      //! Acquires an Eval.
      /*!
        \param eval The Eval to acquire.
      */
      Eval(Eval&& eval);

      //! Acquires an Eval.
      /*!
        \param rhs The Eval to acquire.
      */
      Eval& operator =(Eval&& rhs);

      //! Returns <code>true</code> iff no Async is associated with this Eval.
      bool IsEmpty() const;

      //! Sets the result of this Eval.
      /*!
        \param result The result of this Eval.
      */
      template<typename R>
      void SetResult(R&& result);

      //! Sets the result of this Eval.
      void SetResult();

      virtual void SetException(const std::exception_ptr& e);

      using BaseEval::SetException;
    private:
      friend class Async<T>;
      Async<T>* m_async;

      Eval(RefType<Async<T>> async);
  };
}
}

#endif
