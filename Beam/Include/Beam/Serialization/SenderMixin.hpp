#ifndef BEAM_SENDERMIXIN_HPP
#define BEAM_SENDERMIXIN_HPP
#include <type_traits>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/TypeEntry.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam {
namespace Serialization {

  /*! \class SenderMixin
      \brief Provides default implementations of common Sender methods.
      \tparam SenderType The Sender inheriting this mixin.
   */
  template<typename SenderType>
  class SenderMixin {
    public:

      //! Constructs a SenderMixin with no polymorphic types.
      SenderMixin();

      //! Constructs a SenderMixin.
      /*
        \param registry The TypeRegistry used for sending polymorphic types.
      */
      SenderMixin(Ref<TypeRegistry<SenderType>> registry);

      template<typename T>
      void Shuttle(const T& value);

      template<typename T>
      void Shuttle(const char* name, const T& value);

      template<typename T>
      void Send(const T& value);

      template<typename T>
      typename std::enable_if<std::is_enum<T>::value>::type Send(
        const char* name, T value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<IsStructure<typename std::remove_pointer<
        T>::type>::value>::type Send(const T& value, unsigned int version);

      template<typename T>
      typename std::enable_if<IsStructure<typename std::remove_pointer<
        T>::type>::value && !ImplementsConcept<
        typename std::remove_pointer<T>::type, IO::Buffer>::value>::type Send(
        const char* name, const T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<IsStructure<T>::value>::type Send(
        const char* name, const T& value, unsigned int version);

      template<typename T>
      typename std::enable_if<IsSequence<T>::value>::type Send(const char* name,
        const T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_class<T>::value &&
        !IsStructure<T>::value && !IsSequence<T>::value>::type Send(
        const char* name, const T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<IsStructure<T>::value>::type Send(
        const char* name, T* const& value, unsigned int version);

      template<typename T>
      typename std::enable_if<IsStructure<T>::value>::type Send(
        const char* name, const SerializedValue<T>& value,
        unsigned int version);

    private:
      TypeRegistry<SenderType>* m_typeRegistry;
  };

  template<typename SenderType>
  SenderMixin<SenderType>::SenderMixin()
      : m_typeRegistry(nullptr) {}

  template<typename SenderType>
  SenderMixin<SenderType>::SenderMixin(Ref<TypeRegistry<SenderType>> registry)
      : m_typeRegistry(registry.Get()) {}

  template<typename SenderType>
  template<typename T>
  void SenderMixin<SenderType>::Shuttle(const T& value) {
    static_cast<SenderType*>(this)->Send(value);
  }

  template<typename SenderType>
  template<typename T>
  void SenderMixin<SenderType>::Shuttle(const char* name, const T& value) {
    static_cast<SenderType*>(this)->Send(name, value);
  }

  template<typename SenderType>
  template<typename T>
  void SenderMixin<SenderType>::Send(const T& value) {
    static_cast<SenderType*>(this)->Send(nullptr, value);
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<std::is_enum<T>::value>::type
      SenderMixin<SenderType>::Send(const char* name, T value, void* dummy) {
    static_cast<SenderType*>(this)->Send(name, static_cast<int>(value));
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<IsStructure<typename std::remove_pointer<
      T>::type>::value>::type SenderMixin<SenderType>::Send(const T& value,
      unsigned int version) {
    static_cast<SenderType*>(this)->Send(nullptr, value, version);
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<IsStructure<typename std::remove_pointer<
      T>::type>::value && !ImplementsConcept<
      typename std::remove_pointer<T>::type, IO::Buffer>::value>::type
      SenderMixin<SenderType>::Send(const char* name, const T& value,
      void* dummy) {
    static_cast<SenderType*>(this)->Send(name, value, Version<T>::value);
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<IsStructure<T>::value>::type
      SenderMixin<SenderType>::Send(const char* name, const T& value,
      unsigned int version) {
    static_cast<SenderType*>(this)->StartStructure(name);
    static_cast<SenderType*>(this)->Send("__version", version);
    Serialization::Send<T>()(*static_cast<SenderType*>(this), value, version);
    static_cast<SenderType*>(this)->EndStructure();
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<IsSequence<T>::value>::type
      SenderMixin<SenderType>::Send(const char* name, const T& value,
      void* dummy) {
    static_cast<SenderType*>(this)->StartSequence(name);
    Serialization::Send<T>()(*static_cast<SenderType*>(this), value);
    static_cast<SenderType*>(this)->EndSequence();
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<std::is_class<T>::value && !IsStructure<T>::value &&
      !IsSequence<T>::value>::type SenderMixin<SenderType>::Send(
      const char* name, const T& value, void* dummy) {
    Serialization::Send<T>()(*static_cast<SenderType*>(this), name, value);
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<IsStructure<T>::value>::type
      SenderMixin<SenderType>::Send(const char* name, T* const& value,
      unsigned int version) {
    assert(m_typeRegistry != nullptr);
    static_cast<SenderType*>(this)->StartStructure(name);
    if(value != nullptr) {
      const TypeEntry<SenderType>& entry =
        m_typeRegistry->GetEntry(*value);
      static_cast<SenderType*>(this)->Send("__type", entry.GetName(), 0);
      static_cast<SenderType*>(this)->Send("__version", version);
      entry.Send(*static_cast<SenderType*>(this), value, version);
    } else {
      std::string nullTypeName = "__null";
      static_cast<SenderType*>(this)->Send("__type", nullTypeName, 0);
    }
    static_cast<SenderType*>(this)->EndStructure();
  }

  template<typename SenderType>
  template<typename T>
  typename std::enable_if<IsStructure<T>::value>::type
      SenderMixin<SenderType>::Send(const char* name,
      const SerializedValue<T>& value, unsigned int version) {
    Send(*value);
  }
}
}

#endif
