#ifndef BEAM_RECEIVER_MIXIN_HPP
#define BEAM_RECEIVER_MIXIN_HPP
#include <type_traits>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam::Serialization {

  /**
   * Provides default implementations of common Receiver methods.
   * @param <R> The Receiver inheriting this mixin.
   */
  template<typename R>
  class ReceiverMixin {
    public:

      /** The Receiver inheriting this mixin. */
      using Receiver = R;

      /** Constructs a ReceiverMixin with no polymorphic types. */
      ReceiverMixin();

      /**
       * Constructs a ReceiverMixin.
       * @param registry The TypeRegistry used for receiving polymorphic types.
       */
      ReceiverMixin(
        Ref<const TypeRegistry<typename Inverse<Receiver>::type>> registry);

      template<typename T>
      void Shuttle(T& value, void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_enum_v<T>> Shuttle(const char* name, T& value,
        void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_class_v<T> && IsStructure<T>::value &&
        !ImplementsConcept<T, IO::Buffer>::value> Shuttle(const char* name,
          T& value, void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_class_v<T> && IsSequence<T>::value, int> Shuttle(
        const char* name, T& value, void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_class_v<T> && !IsStructure<T>::value &&
        !IsSequence<T>::value> Shuttle(const char* name, T& value,
          void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_class_v<T>> Shuttle(const char* name, T*& value,
        void* dummy = nullptr);

      template<typename T>
      std::enable_if_t<std::is_class_v<T>> Shuttle(const char* name,
        SerializedValue<T>& value, void* dummy = nullptr);

    private:
      const TypeRegistry<typename Inverse<R>::type>* m_typeRegistry;
  };

  template<typename R>
  ReceiverMixin<R>::ReceiverMixin()
    : m_typeRegistry(nullptr) {}

  template<typename R>
  ReceiverMixin<R>::ReceiverMixin(
    Ref<const TypeRegistry<typename Inverse<R>::type>> registry)
    : m_typeRegistry(registry.Get()) {}

  template<typename R>
  template<typename T>
  void ReceiverMixin<R>::Shuttle(T& value, void* dummy) {
    static_cast<Receiver*>(this)->Shuttle(nullptr, value);
  }

  template<typename R>
  template<typename T>
  std::enable_if_t<std::is_enum_v<T>> ReceiverMixin<R>::Shuttle(
      const char* name, T& value, void* dummy) {
    auto baseValue = std::int32_t();
    static_cast<Receiver*>(this)->Shuttle(name, baseValue);
    value = static_cast<T>(baseValue);
  }

  template<typename R>
  template<typename T>
  std::enable_if_t<std::is_class_v<T> && IsStructure<T>::value &&
      !ImplementsConcept<T, IO::Buffer>::value> ReceiverMixin<R>::Shuttle(
        const char* name, T& value, void* dummy) {
    static_cast<Receiver*>(this)->StartStructure(name);
    unsigned int version;
    static_cast<Receiver*>(this)->Shuttle("__version", version);
    Serialization::Receive<T>()(*static_cast<Receiver*>(this), value, version);
    static_cast<Receiver*>(this)->EndStructure();
  }

  template<typename R>
  template<typename T>
  std::enable_if_t<std::is_class_v<T> && IsSequence<T>::value, int>
      ReceiverMixin<R>::Shuttle(const char* name, T& value, void* dummy) {
    static_cast<Receiver*>(this)->StartSequence(name);
    Serialization::Receive<T>()(*static_cast<Receiver*>(this), value);
    static_cast<Receiver*>(this)->EndSequence();
  }

  template<typename R>
  template<typename T>
  std::enable_if_t<std::is_class_v<T> && !IsStructure<T>::value &&
      !IsSequence<T>::value> ReceiverMixin<R>::Shuttle(const char* name,
        T& value, void* dummy) {
    Serialization::Receive<T>()(*static_cast<Receiver*>(this), name, value);
  }

  template<typename R>
  template<typename T>
  std::enable_if_t<std::is_class_v<T>> ReceiverMixin<R>::Shuttle(
      const char* name, T*& value, void* dummy) {
    assert(m_typeRegistry != nullptr);
    static_cast<Receiver*>(this)->StartStructure(name);
    auto typeName = std::string();
    static_cast<Receiver*>(this)->Shuttle("__type", typeName);
    if(typeName == "__null") {
      value = nullptr;
    } else {
      unsigned int version;
      static_cast<Receiver*>(this)->Shuttle("__version", version);
      auto& entry = m_typeRegistry->GetEntry(typeName);
      value = entry.template Make<T>();
      entry.Receive(*static_cast<Receiver*>(this), value, version);
    }
    static_cast<Receiver*>(this)->EndStructure();
  }

  template<typename R>
  template<typename T>
  std::enable_if_t<std::is_class_v<T>> ReceiverMixin<R>::Shuttle(
      const char* name, SerializedValue<T>& value, void* dummy) {
    value.Initialize();
    Shuttle(name, *value);
  }
}

#endif
