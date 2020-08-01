#ifndef BEAM_WAITABLE_HPP
#define BEAM_WAITABLE_HPP
#include <array>
#include <atomic>
#include <memory>
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /** Base class for an object that performs a blocking operation. */
  class Waitable {
    public:

      /** Constructs a Waitable object. */
      Waitable() = default;

      virtual ~Waitable() = default;

    protected:

      /** Notifies any waiter that this object is available. */
      void Notify();

      /**
       * Returns <code>true</code> iff an otherwise blocking operation will not
       * block on the next invokation.
       */
      virtual bool IsAvailable() const = 0;

    private:
      template<typename... T>
      friend Waitable& Wait(Waitable&, T&...);
      struct WaitCondition {
        std::atomic_int m_counter;
        std::atomic<Waitable*> m_flag;
        ConditionVariable m_isAvailableCondition;
      };
      std::atomic<WaitCondition*> m_condition;

      static void Decrement(WaitCondition* condition);
      void Wait(WaitCondition& condition);
      void Release();
  };

  /**
   * Blocks until a Waitable object becomes available, at which point the next
   * otherwise blocking operation performed on that object is non-blocking.
   * @param waitable The objects to wait on, blocking until one of them becomes
   *        available.
   * @return The object that is available.
   */
  template<typename... T>
  Waitable& Wait(Waitable& a, T&... waitable) {
    auto condition = new Waitable::WaitCondition();
    auto args = std::array{&a, &waitable...};
    condition->m_counter = sizeof...(T) + 2;
    for(auto arg : args) {
      arg->Wait(*condition);
    }
    while(condition->m_flag == nullptr) {
      condition->m_isAvailableCondition.wait();
    }
    auto waitable = condition->m_flag.load();
    for(auto arg : args) {
      arg->Release();
    }
    Waitable::Decrement(condition);
    return *waitable;
  }

  template<typename T, typename... U>
  T& Wait(T& a, U&... waitable) {
    return static_cast<T&>(Wait(static_cast<Waitable&>(a),
      static_cast<Waitable&>(waitable)...));
  }

  inline void Waitable::Notify() {
    auto condition = m_condition.exchange(nullptr);
    if(condition == nullptr) {
      return;
    }
    auto n = static_cast<Waitable*>(nullptr);
    if(condition->m_flag.compare_exchange_strong(n, this)) {
      condition->m_isAvailableCondition.notify_one();
    }
    Decrement(condition);
  }

  inline void Waitable::Decrement(WaitCondition* condition) {
    if(condition->m_counter.fetch_sub(1) == 1) {
      delete condition;
    }
  }

  inline void Waitable::Wait(WaitCondition& condition) {
    m_condition = &condition;
    if(IsAvailable()) {
      Notify();
    }
  }

  inline void Waitable::Release() {
    auto condition = m_condition.exchange(nullptr);
    if(condition == nullptr) {
      return;
    }
    Decrement(condition);
  }
}

#endif
