#ifndef BEAM_ASYNC_INL
#define BEAM_ASYNC_INL
#include <cassert>
#include <utility>
#include "Beam/Routines/Async.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"

namespace Beam::Routines {
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
      Routines::Suspend(Store(m_suspendedRoutines), lock);
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
    Routines::Resume(Store(m_suspendedRoutines));
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
