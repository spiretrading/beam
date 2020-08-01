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
    private:
      struct AvailableTokenDefinition;

    public:

      /** Opaque object used to indicate when an object is available. */
      class AvailableToken {
        private:
          friend struct AvailableTokenDefinition;
          AvailableToken() = default;
      };

      /** Constructs a Waitable object. */
      Waitable() = default;

      virtual ~Waitable() = default;

      /**
       * Returns <code>true</code> iff an otherwise blocking operation will not
       * block on the next invokation.
       */
      virtual bool IsAvailable() const = 0;

      /** Sets the availability token for this object. */
      virtual void SetAvailableToken(AvailableToken& token);

    protected:

      /** Notifies any waiter that this object is available. */
      void Notify();

    private:
      template<typename... T>
      friend Waitable& Wait(Waitable&, T&...);
      struct AvailableTokenDefinition : AvailableToken {
        std::atomic_int m_counter;
        std::atomic<Waitable*> m_flag;
        ConditionVariable m_isAvailableCondition;
      };
      std::atomic<AvailableTokenDefinition*> m_token;

      static void Decrement(AvailableTokenDefinition* token);
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
    auto token = new Waitable::AvailableTokenDefinition();
    auto waitables = std::array{&a, &waitable...};
    token->m_counter = sizeof...(T) + 2;
    for(auto waitable : waitables) {
      waitable->SetAvailableToken(*token);
    }
    while(token->m_flag == nullptr) {
      token->m_isAvailableCondition.wait();
    }
    auto flag = token->m_flag.load();
    for(auto waitable : waitables) {
      waitable->Release();
    }
    Waitable::Decrement(token);
    return *flag;
  }

  template<typename T, typename... U>
  T& Wait(T& a, U&... waitable) {
    return static_cast<T&>(Wait(static_cast<Waitable&>(a),
      static_cast<Waitable&>(waitable)...));
  }

  inline void Waitable::SetAvailableToken(AvailableToken& token) {
    m_token = &static_cast<AvailableTokenDefinition&>(token);
    if(IsAvailable()) {
      Notify();
    }
  }

  inline void Waitable::Notify() {
    auto token = m_token.exchange(nullptr);
    if(token == nullptr) {
      return;
    }
    auto n = static_cast<Waitable*>(nullptr);
    if(token->m_flag.compare_exchange_strong(n, this)) {
      token->m_isAvailableCondition.notify_one();
    }
    Decrement(token);
  }

  inline void Waitable::Decrement(AvailableTokenDefinition* token) {
    if(token->m_counter.fetch_sub(1) == 1) {
      delete token;
    }
  }

  inline void Waitable::Release() {
    auto token = m_token.exchange(nullptr);
    if(!token) {
      return;
    }
    Decrement(token);
  }
}

#endif
