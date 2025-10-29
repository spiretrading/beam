#ifndef BEAM_ASYNC_INL
#define BEAM_ASYNC_INL
#include <cassert>
#include <utility>
#include "Beam/Routines/Async.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"

namespace Beam {
  template<typename T>
  Async<T>::Async() noexcept
    : m_result(std::in_place_index<0>, std::monostate()) {}

  template<typename T>
  Eval<typename Async<T>::Type> Async<T>::get_eval() {
    reset();
    return Eval<typename Async<T>::Type>(Ref(*this));
  }

  template<typename T>
  const std::exception_ptr& Async<T>::get_exception() const {
    if(auto exception = std::get_if<2>(&m_result)) {
      return *exception;
    }
    return BaseAsync::NONE_EXCEPTION;
  }

  template<typename T>
  typename boost::call_traits<typename Async<T>::Type>::reference
      Async<T>::get() {
    auto lock = std::unique_lock(m_mutex);
    while(get_state() == State::PENDING) {
      suspend(out(m_suspended_routines), lock);
    }
    if(get_state() == State::EXCEPTION) {
      std::rethrow_exception(std::get<2>(m_result));
    }
    return std::get<1>(m_result);
  }

  template<typename T>
  BaseAsync::State Async<T>::get_state() const {
    return static_cast<State>(m_result.index());
  }

  template<typename T>
  void Async<T>::reset() {
    if(get_state() == State::PENDING) {
      return;
    }
    m_result.template emplace<0>();
  }

  template<typename T>
  template<typename R>
  void Async<T>::set(R&& result) {
    auto lock = std::lock_guard(m_mutex);
    assert(get_state() == State::PENDING);
    m_result.template emplace<1>(std::forward<R>(result));
    resume(out(m_suspended_routines));
  }

  template<typename T>
  template<typename... A>
  void Async<T>::emplace(A&&... args) {
    auto lock = std::lock_guard(m_mutex);
    assert(get_state() == State::PENDING);
    m_result.template emplace<1>(std::forward<A>(args)...);
    resume(out(m_suspended_routines));
  }

  template<typename T>
  void Async<T>::set_exception(const std::exception_ptr& e) {
    auto lock = std::lock_guard(m_mutex);
    assert(get_state() == State::PENDING);
    m_result.template emplace<2>(e);
    resume(out(m_suspended_routines));
  }

  inline Async<void>::Async() noexcept
    : m_result(std::in_place_index<0>, std::monostate()) {}

  inline Eval<void> Async<void>::get_eval() {
    reset();
    return Eval<void>(Ref(*this));
  }

  inline const std::exception_ptr& Async<void>::get_exception() const {
    if(auto exception = std::get_if<2>(&m_result)) {
      return *exception;
    }
    return BaseAsync::NONE_EXCEPTION;
  }

  inline void Async<void>::get() {
    auto lock = std::unique_lock(m_mutex);
    while(get_state() == State::PENDING) {
      suspend(out(m_suspended_routines), lock);
    }
    if(get_state() == State::EXCEPTION) {
      std::rethrow_exception(std::get<2>(m_result));
    }
  }

  inline BaseAsync::State Async<void>::get_state() const {
    return static_cast<State>(m_result.index());
  }

  inline void Async<void>::reset() {
    if(get_state() == State::PENDING) {
      return;
    }
    m_result.emplace<0>();
  }

  inline void Async<void>::set() {
    auto lock = std::lock_guard(m_mutex);
    assert(get_state() == State::PENDING);
    m_result.emplace<1>();
    resume(out(m_suspended_routines));
  }

  inline void Async<void>::emplace() {
    auto lock = std::lock_guard(m_mutex);
    assert(get_state() == State::PENDING);
    m_result.emplace<1>();
    resume(out(m_suspended_routines));
  }

  inline void Async<void>::set_exception(const std::exception_ptr& e) {
    auto lock = std::lock_guard(m_mutex);
    assert(get_state() == State::PENDING);
    m_result.emplace<2>(e);
    resume(out(m_suspended_routines));
  }

  template<typename E>
  void BaseEval::set_exception(const E& e) {
    set_exception(std::make_exception_ptr(e));
  }

  template<typename T>
  Eval<T>::Eval() noexcept
    : m_async(nullptr) {}

  template<typename T>
  Eval<T>::Eval(Eval&& eval) noexcept {
    *this = std::move(eval);
  }

  template<typename T>
  bool Eval<T>::is_empty() const {
    return !m_async;
  }

  template<typename T>
  template<typename R>
  void Eval<T>::set(R&& result) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    async->set(std::forward<R>(result));
  }

  template<typename T>
  template<typename... R>
  void Eval<T>::emplace(R&&... result) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    async->emplace(std::forward<R>(result)...);
  }

  template<typename T>
  void Eval<T>::set_exception(const std::exception_ptr& e) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    async->set_exception(e);
  }

  template<typename T>
  Eval<T>& Eval<T>::operator =(Eval&& rhs) noexcept {
    if(this == &rhs) {
      return *this;
    }
    m_async = rhs.m_async;
    rhs.m_async = nullptr;
    return *this;
  }

  template<typename T>
  Eval<T>::Eval(Ref<Async<Type>> async)
    : m_async(async.get()) {}

  inline Eval<void>::Eval() noexcept
    : m_async(nullptr) {}

  inline Eval<void>::Eval(Eval&& eval) noexcept {
    *this = std::move(eval);
  }

  inline bool Eval<void>::is_empty() const {
    return !m_async;
  }

  inline void Eval<void>::set() {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    async->set();
  }

  inline void Eval<void>::emplace() {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    async->emplace();
  }

  inline void Eval<void>::set_exception(const std::exception_ptr& e) {
    if(!m_async) {
      return;
    }
    auto async = std::exchange(m_async, nullptr);
    async->set_exception(e);
  }

  inline Eval<void>& Eval<void>::operator =(Eval&& rhs) noexcept {
    if(this == &rhs) {
      return *this;
    }
    m_async = rhs.m_async;
    rhs.m_async = nullptr;
    return *this;
  }

  inline Eval<void>::Eval(Ref<Async<Type>> async)
    : m_async(async.get()) {}
}

#endif
