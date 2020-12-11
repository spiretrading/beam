#ifndef BEAM_SENDER_MIXIN_HPP
#define BEAM_SENDER_MIXIN_HPP
#include <type_traits>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/TypeEntry.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam::Serialization {

  /**
   * Provides default implementations of common Sender methods.
   * @param <S> The Sender inheriting this mixin.
   */
  template<typename S>
  class SenderMixin {
    public:

      /** The type of Sender this is being mixed into. */
      using Sender = S;

      /** Constructs a SenderMixin with no polymorphic types. */
      SenderMixin();

      /**
       * Constructs a SenderMixin.
       * @param registry The TypeRegistry used for sending polymorphic types.
       */
      SenderMixin(Ref<const TypeRegistry<Sender>> registry);

      template<typename T>
      void Shuttle(const T& value);

      template<typename T>
      void Shuttle(const char* name, const T& value);

      template<typename T>
      void Send(const T& value);

      template<typename T>
      std::enable_if_t<std::is_enum_v<T>> Send(const char* name, T value,
        void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<IsStructure<std::remove_pointer_t<T>>::value> Send(
        const T& value, unsigned int version);

      template<typename T>
      std::enable_if_t<IsStructure<std::remove_pointer_t<T>>::value &&
        !ImplementsConcept<std::remove_pointer_t<T>, IO::Buffer>::value> Send(
          const char* name, const T& value, void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<IsStructure<T>::value> Send(const char* name,
        const T& value, unsigned int version);

      template<typename T>
      std::enable_if_t<IsSequence<T>::value> Send(const char* name,
        const T& value, void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_class_v<T> && !IsStructure<T>::value &&
        !IsSequence<T>::value> Send(const char* name, const T& value,
          void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<IsStructure<T>::value> Send(const char* name,
        T* const& value, unsigned int version);

      template<typename T>
      std::enable_if_t<IsStructure<T>::value> Send(const char* name,
        const SerializedValue<T>& value, unsigned int version);

    private:
      const TypeRegistry<Sender>* m_typeRegistry;
  };

  template<typename S>
  SenderMixin<S>::SenderMixin()
    : m_typeRegistry(nullptr) {}

  template<typename S>
  SenderMixin<S>::SenderMixin(Ref<const TypeRegistry<Sender>> registry)
    : m_typeRegistry(registry.Get()) {}

  template<typename S>
  template<typename T>
  void SenderMixin<S>::Shuttle(const T& value) {
    static_cast<Sender*>(this)->Send(value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::Shuttle(const char* name, const T& value) {
    static_cast<Sender*>(this)->Send(name, value);
  }

  template<typename S>
  template<typename T>
  void SenderMixin<S>::Send(const T& value) {
    static_cast<Sender*>(this)->Send(nullptr, value);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_enum_v<T>> SenderMixin<S>::Send(const char* name,
      T value, void* dummy) {
    static_cast<Sender*>(this)->Send(name, static_cast<int>(value));
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<IsStructure<std::remove_pointer_t<T>>::value>
      SenderMixin<S>::Send(const T& value, unsigned int version) {
    static_cast<Sender*>(this)->Send(nullptr, value, version);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<IsStructure<std::remove_pointer_t<T>>::value &&
      !ImplementsConcept<std::remove_pointer_t<T>, IO::Buffer>::value>
        SenderMixin<S>::Send(const char* name, const T& value, void* dummy) {
    static_cast<Sender*>(this)->Send(name, value, Version<T>::value);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<IsStructure<T>::value> SenderMixin<S>::Send(const char* name,
      const T& value, unsigned int version) {
    static_cast<Sender*>(this)->StartStructure(name);
    static_cast<Sender*>(this)->Send("__version", version);
    Serialization::Send<T>()(*static_cast<Sender*>(this), value, version);
    static_cast<Sender*>(this)->EndStructure();
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<IsSequence<T>::value> SenderMixin<S>::Send(const char* name,
      const T& value, void* dummy) {
    static_cast<Sender*>(this)->StartSequence(name);
    Serialization::Send<T>()(*static_cast<Sender*>(this), value);
    static_cast<Sender*>(this)->EndSequence();
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<std::is_class_v<T> && !IsStructure<T>::value &&
      !IsSequence<T>::value> SenderMixin<S>::Send(const char* name,
        const T& value, void* dummy) {
    Serialization::Send<T>()(*static_cast<Sender*>(this), name, value);
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<IsStructure<T>::value> SenderMixin<S>::Send(const char* name,
      T* const& value, unsigned int version) {
    assert(m_typeRegistry != nullptr);
    static_cast<Sender*>(this)->StartStructure(name);
    if(value) {
      auto& entry = m_typeRegistry->GetEntry(*value);
      static_cast<Sender*>(this)->Send("__type", entry.GetName(), 0);
      static_cast<Sender*>(this)->Send("__version", version);
      entry.Send(*static_cast<Sender*>(this), value, version);
    } else {
      static const auto NULL_TYPE_NAME = std::string("__null");
      static_cast<Sender*>(this)->Send("__type", NULL_TYPE_NAME, 0);
    }
    static_cast<Sender*>(this)->EndStructure();
  }

  template<typename S>
  template<typename T>
  std::enable_if_t<IsStructure<T>::value> SenderMixin<S>::Send(const char* name,
      const SerializedValue<T>& value, unsigned int version) {
    Send(*value);
  }
}

#endif
