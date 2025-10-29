#ifndef BEAM_SERVICE_RESULT_HPP
#define BEAM_SERVICE_RESULT_HPP
#include <exception>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Async.hpp"

namespace Beam::Tests {

  /** Abstract base for capturing the result of a service invocation. */
  class BaseServiceResult {
    public:
      virtual ~BaseServiceResult() = default;

      /**
       * Sets an error for the service result.
       * @param e The exception representing the error.
       */
      virtual void set(const std::exception_ptr& e) = 0;

    protected:

      /** Constructs an empty result. */
      BaseServiceResult() = default;
  };

  /**
   * Captures and delivers an asynchronous service result of type T.
   * @tparam T The type of the result value.
   */
  template<typename T>
  class ServiceResult : public BaseServiceResult {
    public:

      /** The type of the result value. */
      using Result = T;

      /**
       * Constructs the ServiceResult with an evaluation callback.
       * @param result Stores the result or error of the service call.
       */
      ServiceResult(Eval<Result> result) noexcept;

      /**
       * Sets the successful result value.
       * @param result The result value.
       */
      void set(const Result& result);

      /**
       * Sets the error result value.
       * @param e The error.
       */
      void set(const std::exception_ptr& e) override;

    private:
      mutable boost::mutex m_mutex;
      bool m_is_set;
      Eval<Result> m_result;
  };

  /** Specializes ServiceResult for operations with no return value. */
  template<>
  class ServiceResult<void> : public BaseServiceResult {
    public:

      /** The type of the result value. */
      using Result = void;

      /**
       * Constructs the ServiceResult with an evaluation callback.
       * @param result Stores the result or error of the service call.
       */
      ServiceResult(Eval<Result> result) noexcept;

      /** Marks the operation as a success. */
      void set();

      /**
       * Sets the error result value.
       * @param e The error.
       */
      void set(const std::exception_ptr& e) override;

    private:
      mutable boost::mutex m_mutex;
      bool m_is_set;
      Eval<Result> m_result;
  };

  template<typename T>
  ServiceResult<T>::ServiceResult(Eval<Result> result) noexcept
    : m_is_set(false),
      m_result(std::move(result)) {}

  template<typename T>
  void ServiceResult<T>::set(const Result& result) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_is_set) {
      return;
    }
    m_is_set = true;
    m_result.set(result);
  }

  template<typename T>
  void ServiceResult<T>::set(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_is_set) {
      return;
    }
    m_is_set = true;
    m_result.set_exception(e);
  }

  inline ServiceResult<void>::ServiceResult(Eval<Result> result) noexcept
    : m_is_set(false),
      m_result(std::move(result)) {}

  inline void ServiceResult<void>::set() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_is_set) {
      return;
    }
    m_is_set = true;
    m_result.set();
  }

  inline void ServiceResult<void>::set(const std::exception_ptr& e) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_is_set) {
      return;
    }
    m_is_set = true;
    m_result.set_exception(e);
  }
}

#endif
