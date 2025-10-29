#ifndef BEAM_RECEIVER_MIXIN_HPP
#define BEAM_RECEIVER_MIXIN_HPP
#include <typeindex>
#include <type_traits>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/SerializedValue.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam {

  /**
   * Provides default implementations of common Receiver methods.
   * @tparam R The Receiver inheriting this mixin.
   */
  template<typename R>
  class ReceiverMixin {
    public:

      /** The Receiver inheriting this mixin. */
      using Receiver = R;

      /** Constructs a ReceiverMixin with no polymorphic types. */
      ReceiverMixin() noexcept;

      /**
       * Constructs a ReceiverMixin.
       * @param registry The TypeRegistry used for receiving polymorphic types.
       */
      explicit ReceiverMixin(
        Ref<const TypeRegistry<inverse_t<Receiver>>> registry) noexcept;

      template<typename T>
      void shuttle(const char* name, const T& value);
      template<typename T>
      void shuttle(const T& value);
      template<typename T>
      void receive(T& value);
      void receive(const char* name, std::type_index& value);
      template<typename T>
      void receive(const char* name, T& value) requires std::is_enum_v<T>;
      template<typename T>
      void receive(const char* name, T& value) requires(
        std::is_class_v<T> && is_structure<T> && !IsBuffer<T>);
      template<typename T>
      void receive(const char* name, T& value) requires(
        std::is_class_v<T> && is_sequence<T> && !IsBuffer<T>);
      template<typename T>
      void receive(const char* name, T& value) requires(std::is_class_v<T> &&
        !is_structure<T> && !is_sequence<T> && !IsBuffer<T>);
      template<typename T>
      void receive(const char* name, T*& value);
      template<typename T>
      void receive(const char* name, T*& value) requires std::is_class_v<T>;

    private:
      const TypeRegistry<inverse_t<R>>* m_registry;

      Receiver& self();
  };

  template<typename R>
  ReceiverMixin<R>::ReceiverMixin() noexcept
    : m_registry(nullptr) {}

  template<typename R>
  ReceiverMixin<R>::ReceiverMixin(
    Ref<const TypeRegistry<inverse_t<R>>> registry) noexcept
    : m_registry(registry.get()) {}

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::shuttle(const char* name, const T& value) {
    self().receive(name, const_cast<T&>(value));
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::shuttle(const T& value) {
    self().receive(const_cast<T&>(value));
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(T& value) {
    self().receive(nullptr, value);
  }

  template<typename R>
  void ReceiverMixin<R>::receive(const char* name, std::type_index& value) {
    assert(m_registry != nullptr);
    auto type_name = std::string();
    self().receive(name, type_name);
    value = m_registry->get_type_index(type_name);
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(const char* name, T& value) requires
      std::is_enum_v<T> {
    auto base_value = std::int32_t();
    self().receive(name, base_value);
    value = static_cast<T>(base_value);
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(const char* name, T& value) requires(
      std::is_class_v<T> && is_structure<T> && !IsBuffer<T>) {
    self().start_structure(name);
    auto version = unsigned();
    self().receive("__version", version);
    Receive<T>()(self(), value, version);
    self().end_structure();
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(const char* name, T& value) requires(
      std::is_class_v<T> && is_sequence<T> && !IsBuffer<T>) {
    self().start_sequence(name);
    Receive<T>()(self(), value);
    self().end_sequence();
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(const char* name, T& value) requires(
      std::is_class_v<T> && !is_structure<T> && !is_sequence<T> &&
        !IsBuffer<T>) {
    Receive<T>()(self(), name, value);
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(const char* name, T*& value) {
    value = DataShuttle::make_new<T>();
    self().receive(name, *value);
  }

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::receive(const char* name, T*& value) requires
      std::is_class_v<T> {
    assert(m_registry != nullptr);
    self().start_structure(name);
    auto type_name = std::string();
    self().receive("__type", type_name);
    if(type_name == "__null") {
      value = nullptr;
    } else {
      auto version = unsigned();
      self().receive("__version", version);
      auto& entry = m_registry->get_entry(type_name);
      value = static_cast<T*>(entry.make());
      entry.receive(self(), value, version);
    }
    self().end_structure();
  }

  template<typename R>
  typename ReceiverMixin<R>::Receiver& ReceiverMixin<R>::self() {
    return static_cast<Receiver&>(*this);
  }
}

#endif
