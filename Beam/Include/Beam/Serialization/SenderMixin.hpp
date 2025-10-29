#ifndef BEAM_SENDER_MIXIN_HPP
#define BEAM_SENDER_MIXIN_HPP
#include <typeindex>
#include <type_traits>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/TypeEntry.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam {

  /**
   * Provides default implementations of common Sender methods.
   * @tparam S The Sender inheriting this mixin.
   */
  template<typename S>
  class SenderMixin {
    public:

      /** The type of Sender this is being mixed into. */
      using Sender = S;

      /** Constructs a SenderMixin with no polymorphic types. */
      SenderMixin() noexcept;

      /**
       * Constructs a SenderMixin.
       * @param registry The TypeRegistry used for sending polymorphic types.
       */
      explicit SenderMixin(Ref<const TypeRegistry<Sender>> registry) noexcept;

      template<typename T>
      void shuttle(const char* name, const T& value);
      template<typename T>
      void shuttle(const T& value);
      template<typename T>
      void send(const T& value);
      template<typename T>
      void send_version(const T& value, unsigned int version);
      void send(const char* name, std::type_index value);
      template<typename T> requires std::is_enum_v<T>
      void send(const char* name, T value);
      template<typename T>
      void send(const char* name, const T& value) requires(
        std::is_class_v<T> && is_structure<T> && !IsConstBuffer<T>);
      template<typename T>
      void send_version(
        const char* name, const T& value, unsigned int version) requires(
          std::is_class_v<T> && is_structure<T> && !IsConstBuffer<T>);
      template<typename T>
      void send(const char* name, const T& value) requires(
        std::is_class_v<T> && is_sequence<T> && !IsConstBuffer<T>);
      template<typename T>
      void send(const char* name, const T& value) requires(std::is_class_v<T> &&
        !is_structure<T> && !is_sequence<T> && !IsConstBuffer<T>);
      template<typename T>
      void send(const char* name, const T* value);
      template<typename T>
      void send(const char* name, const T* value) requires std::is_class_v<T>;
      template<typename T>
      void send_version(const char* name, const T* value, unsigned int version);
      template<typename T>
      void send_version(const char* name, const T* value, unsigned int version)
        requires std::is_class_v<T>;
      template<typename T>
      void send(const char* name, const SerializedValue<T>& value);

    private:
      const TypeRegistry<Sender>* m_registry;

      Sender& self();
  };

  template<typename S>
  SenderMixin<S>::SenderMixin() noexcept
    : m_registry(nullptr) {}

  template<typename S>
  SenderMixin<S>::SenderMixin(Ref<const TypeRegistry<Sender>> registry) noexcept
    : m_registry(registry.get()) {}

  template<typename S>
  template<typename T>
  void SenderMixin<S>::shuttle(const T& value) {
    self().send(value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::shuttle(const char* name, const T& value) {
    self().send(name, value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const T& value) {
    self().send(nullptr, value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send_version(const T& value, unsigned int version) {
    self().send_version(nullptr, value, version);
  }

  template<typename S>
  void SenderMixin<S>::send(const char* name, std::type_index value) {
    assert(m_registry != nullptr);
    self().send(name, m_registry->get_type_name(value));
  }

  template<typename S>
  template<typename T> requires std::is_enum_v<T>
  void SenderMixin<S>::send(const char* name, T value) {
    self().send(name, static_cast<std::int32_t>(value));
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const char* name, const T& value) requires(
      std::is_class_v<T> && is_structure<T> && !IsConstBuffer<T>) {
    self().send_version(name, value, shuttle_version<T>);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send_version(
      const char* name, const T& value, unsigned int version) requires(
        std::is_class_v<T> && is_structure<T> && !IsConstBuffer<T>) {
    self().start_structure(name);
    self().send("__version", version);
    Send<T>()(self(), value, version);
    self().end_structure();
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const char* name, const T& value) requires(
      std::is_class_v<T> && is_sequence<T> && !IsConstBuffer<T>) {
    self().start_sequence(name);
    Send<T>()(self(), value);
    self().end_sequence();
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const char* name, const T& value) requires(
      std::is_class_v<T> && !is_structure<T> && !is_sequence<T> &&
        !IsConstBuffer<T>) {
    Send<T>()(self(), name, value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const char* name, const T* value) {
    self().send(name, *value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const char* name, const T* value)
      requires std::is_class_v<T> {
    self().send_version(name, value, shuttle_version<T>);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send_version(
      const char* name, const T* value, unsigned int version) {
    self().send_version(name, *value, version);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send_version(
      const char* name, const T* value, unsigned int version)
        requires std::is_class_v<T> {
    assert(m_registry != nullptr);
    self().start_structure(name);
    if(value) {
      auto& entry = m_registry->get_entry(*value);
      self().send("__type", entry.get_name());
      self().send("__version", version);
      entry.send(self(), value, version);
    } else {
      static const auto NULL_TYPE_NAME = std::string("__null");
      self().send("__type", NULL_TYPE_NAME);
    }
    self().end_structure();
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::send(const char* name, const SerializedValue<T>& value) {
    send(*value);
  }

  template<typename S>
  typename SenderMixin<S>::Sender& SenderMixin<S>::self() {
    return *static_cast<Sender*>(this);
  }
}

#endif
