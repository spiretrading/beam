#ifndef BEAM_TYPE_REGISTRY_HPP
#define BEAM_TYPE_REGISTRY_HPP
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/TypeEntry.hpp"
#include "Beam/Serialization/TypeNotFoundException.hpp"

#define BEAM_GET_TYPE_NAME(Name, ...) Name
#define BEAM_GET_TYPE_UID(Name, Uid, ...) Uid

#define BEAM_REGISTER_TYPE(z, n, q)                                            \
  registry->template Register<BEAM_GET_TYPE_NAME q>(BEAM_GET_TYPE_UID q);

#define BEAM_REGISTER_TYPES_(Name, TypeList)                                   \
  template<typename S>                                                         \
  void Name(::Beam::Out< ::Beam::Serialization::TypeRegistry<S>> registry) {   \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_TYPE, BOOST_PP_EMPTY, TypeList);      \
  }

#define BEAM_REGISTER_TYPES(Name, ...)                                         \
  BEAM_REGISTER_TYPES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),      \
    (__VA_ARGS__)))

namespace Beam::Serialization {
namespace Details {
  struct NullShuttle {
    template<typename Shuttler>
    void Shuttle(Shuttler&, unsigned int version) {}
  };
}

  /**
   * Stores a registry of polymorphic types capable of serialization.
   * @param <S> The type of Sender.
   */
  template<typename S>
  class TypeRegistry {
    public:

      /** Specifies the Sender's type. */
      using Sender = S;

      /** Specifies the Receiver's type. */
      using Receiver = typename Inverse<S>::type;

      /** The TypeEntries stored by this registry. */
      using TypeEntry = Serialization::TypeEntry<Sender>;

      /** Constructs a TypeRegistry. */
      TypeRegistry();

      /** Copies a TypeRegistry. */
      TypeRegistry(const TypeRegistry& registry);

      /** Returns the TypeEntry for a specified type. */
      template<typename T>
      const TypeEntry& GetEntry() const;

      /** Returns the TypeEntry for a given value. */
      template<typename T>
      const TypeEntry& GetEntry(const T& value) const;

      /**
       * Returns the TypeEntry with a given name.
       * @param name The name of the TypeEntry.
       * @return The TypeEntry with the specified <i>name</i>.
       */
      const TypeEntry& GetEntry(const std::string& name) const;

      /**
       * Registers a type.
       * @param <T> The type to register.
       * @param name The name of the type to register.
       */
      template<typename T>
      void Register(const std::string& name);

      /**
       * Adds all types from an existing registry.
       * @param registry The TypeRegistry to add.
       */
      void Add(const TypeRegistry& registry);

    private:
      using TypeEntryIterator =
        typename std::unordered_map<std::type_index, TypeEntry>::iterator;
      std::unordered_map<std::type_index, TypeEntry> m_types;
      std::unordered_map<std::string, TypeEntryIterator> m_typeNames;

      template<typename T>
      static void Send(Sender& sender, void* value, unsigned int version);
      template<typename T>
      static void Receive(Receiver& receiver, void* value,
        unsigned int version);
  };

  template<typename S>
  TypeRegistry<S>::TypeRegistry() {
    Register<Details::NullShuttle>("__null");
  }

  template<typename S>
  TypeRegistry<S>::TypeRegistry(const TypeRegistry& registry) {
    Add(registry);
  }

  template<typename S>
  template<typename T>
  const TypeEntry<S>& TypeRegistry<S>::GetEntry() const {
    auto type = std::type_index(typeid(T));
    auto typeIterator = m_types.find(type);
    if(typeIterator == m_types.end()) {
      BOOST_THROW_EXCEPTION(TypeNotFoundException(type.name()));
    }
    return typeIterator->second;
  }

  template<typename S>
  template<typename T>
  const TypeEntry<S>& TypeRegistry<S>::GetEntry(const T& value) const {
    auto type = std::type_index(typeid(*(&const_cast<T&>(value))));
    auto typeIterator = m_types.find(type);
    if(typeIterator == m_types.end()) {
      BOOST_THROW_EXCEPTION(TypeNotFoundException(type.name()));
    }
    return typeIterator->second;
  }

  template<typename S>
  const TypeEntry<S>& TypeRegistry<S>::GetEntry(const std::string& name) const {
    auto typeIterator = m_typeNames.find(name);
    if(typeIterator == m_typeNames.end()) {
      BOOST_THROW_EXCEPTION(TypeNotFoundException(name));
    }
    return typeIterator->second->second;
  }

  template<typename S>
  template<typename T>
  void TypeRegistry<S>::Register(const std::string& name) {
    auto builder = static_cast<T* (*)()>(&DataShuttle::Builder<T>);
    auto sender = &Send<T>;
    auto receiver = &Receive<T>;
    auto type = std::type_index(typeid(T));
    auto entry = TypeEntry(type, name, std::move(builder), std::move(sender),
      std::move(receiver));
    auto insertResult = m_types.insert(std::pair(type, std::move(entry)));
    if(insertResult.second) {
      m_typeNames.insert(std::pair(name, insertResult.first));
    }
  }

  template<typename S>
  void TypeRegistry<S>::Add(const TypeRegistry& registry) {
    for(auto& typeEntry : registry.m_typeNames) {
      auto entry = TypeEntry(typeEntry.second->second);
      auto type = std::type_index(entry.GetType());
      auto insertResult = m_types.insert(std::pair(type, std::move(entry)));
      if(insertResult.second) {
        m_typeNames.insert(std::pair(typeEntry.first, insertResult.first));
      }
    }
  }

  template<typename S>
  template<typename T>
  void TypeRegistry<S>::Send(Sender& sender, void* value,
      unsigned int version) {
    Serialization::Send<T>()(sender, *static_cast<T*>(value), version);
  }

  template<typename S>
  template<typename T>
  void TypeRegistry<S>::Receive(Receiver& receiver, void* value,
      unsigned int version) {
    Serialization::Receive<T>()(receiver, *static_cast<T*>(value), version);
  }
}

#endif
