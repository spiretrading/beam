#ifndef BEAM_ACTIVE_HPP
#define BEAM_ACTIVE_HPP
#include <atomic>
#include <concepts>
#include <memory>
#include <type_traits>

namespace Beam {

  /**
   * Encapsulates an object whose updates result in newly created values.
   * @tparam T The type of value to store.
   */
  template<typename T>
  class Active {
    public:

      /** The type of value to store. */
      using Type = T;

      /**
       * Constructs an Active object.
       * @param args The parameters forwarded to the value's constructor.
       */
      template<typename... U> requires std::constructible_from<T, U...>
      Active(U&&... args) noexcept(std::is_nothrow_constructible_v<T, U...>);

      /** Returns the currently stored value. */
      std::shared_ptr<const Type> load() const;

      /** Returns the currently stored value. */
      std::shared_ptr<Type> load();

      /**
       * Updates the value stored.
       * @param value The new value to store.
       */
      template<typename... U> requires std::constructible_from<T, U...>
      void update(U&&... value);

    private:
      std::atomic<std::shared_ptr<Type>> m_value;

      Active(const Active&) = delete;
      Active& operator =(const Active&) = delete;
  };

  template<typename U>
  Active(U&&) -> Active<std::remove_cvref_t<U>>;

  template<typename T>
  template<typename... U> requires std::constructible_from<T, U...>
  Active<T>::Active(U&&... args)
    noexcept(std::is_nothrow_constructible_v<T, U...>)
    : m_value(std::make_shared<Type>(std::forward<U>(args)...)) {}

  template<typename T>
  std::shared_ptr<const typename Active<T>::Type> Active<T>::load() const {
    return m_value.load(std::memory_order_acquire);
  }

  template<typename T>
  std::shared_ptr<typename Active<T>::Type> Active<T>::load() {
    return m_value.load(std::memory_order_acquire);
  }

  template<typename T>
  template<typename... U> requires std::constructible_from<T, U...>
  void Active<T>::update(U&&... value) {
    m_value.store(std::make_shared<Type>(std::forward<U>(value)...),
      std::memory_order_release);
  }
}

#endif
