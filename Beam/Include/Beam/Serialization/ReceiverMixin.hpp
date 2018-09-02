#ifndef BEAM_RECEIVERMIXIN_HPP
#define BEAM_RECEIVERMIXIN_HPP
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam {
namespace Serialization {

  /*! \class ReceiverMixin
      \brief Provides default implementations of common Receiver methods.
      \tparam ReceiverType The Receiver inheriting this mixin.
   */
  template<typename ReceiverType>
  class ReceiverMixin {
    public:

      //! Constructs a ReceiverMixin with no polymorphic types.
      ReceiverMixin();

      //! Constructs a ReceiverMixin.
      /*
        \param registry The TypeRegistry used for receiving polymorphic types.
      */
      ReceiverMixin(Ref<TypeRegistry<typename Inverse<ReceiverType>::type>>
        registry);

      template<typename T>
      void Shuttle(T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_enum<T>::value>::type Shuttle(
        const char* name, T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_class<T>::value &&
        IsStructure<T>::value && !ImplementsConcept<T, IO::Buffer>::value>::type
        Shuttle(const char* name, T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_class<T>::value &&
        IsSequence<T>::value, int>::type Shuttle(const char* name, T& value,
        void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_class<T>::value &&
        !IsStructure<T>::value && !IsSequence<T>::value>::type Shuttle(
        const char* name, T& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_class<T>::value>::type Shuttle(
        const char* name, T*& value, void* dummy = nullptr);

      template<typename T>
      typename std::enable_if<std::is_class<T>::value>::type Shuttle(
        const char* name, SerializedValue<T>& value, void* dummy = nullptr);

    private:
      TypeRegistry<typename Inverse<ReceiverType>::type>* m_typeRegistry;
  };

  template<typename ReceiverType>
  ReceiverMixin<ReceiverType>::ReceiverMixin()
      : m_typeRegistry(nullptr) {}

  template<typename ReceiverType>
  ReceiverMixin<ReceiverType>::ReceiverMixin(Ref<TypeRegistry<
      typename Inverse<ReceiverType>::type>> registry)
      : m_typeRegistry(registry.Get()) {}

  template<typename ReceiverType>
  template<typename T>
  void ReceiverMixin<ReceiverType>::Shuttle(T& value, void* dummy) {
    static_cast<ReceiverType*>(this)->Shuttle(nullptr, value);
  }

  template<typename ReceiverType>
  template<typename T>
  typename std::enable_if<std::is_enum<T>::value>::type
      ReceiverMixin<ReceiverType>::Shuttle(const char* name, T& value,
      void* dummy) {
    std::int32_t baseValue;
    static_cast<ReceiverType*>(this)->Shuttle(name, baseValue);
    value = static_cast<T>(baseValue);
  }

  template<typename ReceiverType>
  template<typename T>
  typename std::enable_if<std::is_class<T>::value && IsStructure<T>::value &&
      !ImplementsConcept<T, IO::Buffer>::value>::type
      ReceiverMixin<ReceiverType>::Shuttle(const char* name, T& value,
      void* dummy) {
    static_cast<ReceiverType*>(this)->StartStructure(name);
    unsigned int version;
    static_cast<ReceiverType*>(this)->Shuttle("__version", version);
    Serialization::Receive<T>()(*static_cast<ReceiverType*>(this), value,
      version);
    static_cast<ReceiverType*>(this)->EndStructure();
  }

  template<typename ReceiverType>
  template<typename T>
  typename std::enable_if<std::is_class<T>::value &&
      IsSequence<T>::value, int>::type ReceiverMixin<ReceiverType>::Shuttle(
      const char* name, T& value, void* dummy) {
    static_cast<ReceiverType*>(this)->StartSequence(name);
    Serialization::Receive<T>()(*static_cast<ReceiverType*>(this), value);
    static_cast<ReceiverType*>(this)->EndSequence();
  }

  template<typename ReceiverType>
  template<typename T>
  typename std::enable_if<std::is_class<T>::value && !IsStructure<T>::value &&
      !IsSequence<T>::value>::type ReceiverMixin<ReceiverType>::Shuttle(
      const char* name, T& value, void* dummy) {
    Serialization::Receive<T>()(*static_cast<ReceiverType*>(this), name, value);
  }

  template<typename ReceiverType>
  template<typename T>
  typename std::enable_if<std::is_class<T>::value>::type
      ReceiverMixin<ReceiverType>::Shuttle(const char* name, T*& value,
      void* dummy) {
    assert(m_typeRegistry != nullptr);
    static_cast<ReceiverType*>(this)->StartStructure(name);
    std::string typeName;
    static_cast<ReceiverType*>(this)->Shuttle("__type", typeName);
    if(typeName == "__null") {
      value = nullptr;
    } else {
      unsigned int version;
      static_cast<ReceiverType*>(this)->Shuttle("__version", version);
      const TypeEntry<typename Inverse<ReceiverType>::type>& entry =
        m_typeRegistry->GetEntry(typeName);
      value = entry.template Build<T>();
      entry.Receive(*static_cast<ReceiverType*>(this), value, version);
    }
    static_cast<ReceiverType*>(this)->EndStructure();
  }

  template<typename ReceiverType>
  template<typename T>
  typename std::enable_if<std::is_class<T>::value>::type
      ReceiverMixin<ReceiverType>::Shuttle(const char* name,
      SerializedValue<T>& value, void* dummy) {
    value.Initialize();
    Shuttle(name, *value);
  }
}
}

#endif
