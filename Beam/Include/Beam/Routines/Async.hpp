#ifndef BEAM_ASYNC_HPP
#define BEAM_ASYNC_HPP
#include <exception>
#include <variant>
#include <boost/call_traits.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"

namespace Beam {
  template<typename> class Eval;

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
      inline static const auto NONE_EXCEPTION = std::exception_ptr();

      /** Constructs an empty BaseAsync. */
      BaseAsync() = default;

    private:
      BaseAsync(const BaseAsync&) = delete;
      BaseAsync& operator =(const BaseAsync&) = delete;
  };

  /**
   * Stores the result of an asynchronous operation.
   * @tparam T The type of the asynchronous result.
   */
  template<typename T>
  class Async final : public BaseAsync {
    public:

      /** The type of the asynchronous result. */
      using Type = T;

      /** Constructs an Async. */
      Async() noexcept;

      /** Returns an object that can be used to evaluate this Async. */
      Eval<Type> get_eval();

      /** Returns the result of the operation. */
      typename boost::call_traits<Type>::reference get();

      /** Returns the exception. */
      const std::exception_ptr& get_exception() const;

      /** Returns the state of this Async. */
      State get_state() const;

      /** Resets this Async so that it can be reused. */
      void reset();

    private:
      friend class Eval<Type>;
      mutable boost::mutex m_mutex;
      SuspendedRoutineQueue m_suspended_routines;
      std::variant<std::monostate, Type, std::exception_ptr> m_result;

      template<typename R>
      void set(R&& result);
      template<typename... A>
      void emplace(A&&... args);
      void set_exception(const std::exception_ptr& e);
  };

  template<>
  class Async<void> final : public BaseAsync {
    public:
      using Type = void;

      Async() noexcept;

      Eval<Type> get_eval();
      void get();
      const std::exception_ptr& get_exception() const;
      State get_state() const;
      void reset();

    private:
      friend class Eval<Type>;
      mutable boost::mutex m_mutex;
      SuspendedRoutineQueue m_suspended_routines;
      std::variant<
        std::monostate, std::monostate, std::exception_ptr> m_result;

      void set();
      void emplace();
      void set_exception(const std::exception_ptr& e);
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
      void set_exception(const E& e);

      /**
       * Sets an exception to be returned.
       * @param e The exception to set.
       */
      virtual void set_exception(const std::exception_ptr& e) = 0;

    protected:

      /** Constructs an empty BaseEval. */
      BaseEval() = default;

    private:
      BaseEval(const BaseEval&) = delete;
      BaseEval& operator =(const BaseEval&) = delete;
  };

  /**
   * Evaluates the result of an asynchronous operation.
   * @tparam T The type of the asynchronous operation.
   */
  template<typename T>
  class Eval final : public BaseEval {
    public:

      /** The type of the asynchronous result. */
      using Type = T;

      /** Constructs an Eval whose result is discarded. */
      Eval() noexcept;

      Eval(Eval&& eval) noexcept;

      /**
       * Returns <code>true</code> iff no Async is associated with this Eval.
       */
      bool is_empty() const;

      /**
       * Sets the result of this Eval.
       * @param result The result of this Eval.
       */
      template<typename R>
      void set(R&& result);

      /** Emplaces the result of this Eval. */
      template<typename... R>
      void emplace(R&&... result);

      void set_exception(const std::exception_ptr& e) override;
      Eval& operator =(Eval&& rhs) noexcept;
      using BaseEval::set_exception;

    private:
      friend class Async<Type>;
      Async<Type>* m_async;

      Eval(Ref<Async<Type>> async);
  };

  template<>
  class Eval<void> final : public BaseEval {
    public:
      using Type = void;

      Eval() noexcept;
      Eval(Eval&& eval) noexcept;

      bool is_empty() const;
      void set();
      void emplace();
      void set_exception(const std::exception_ptr& e) override;
      Eval& operator =(Eval&& rhs) noexcept;
      using BaseEval::set_exception;

    private:
      friend class Async<Type>;
      Async<Type>* m_async;

      Eval(Ref<Async<Type>> async);
  };
}

#include "Beam/Routines/Routine.hpp"

#endif
