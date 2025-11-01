#ifndef BEAM_TYPE_REGISTRY_HPP
#define BEAM_TYPE_REGISTRY_HPP
#include <functional>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/TypeEntry.hpp"
#include "Beam/Serialization/TypeNotFoundException.hpp"

#define BEAM_GET_TYPE_NAME(Name, ...) Name
#define BEAM_GET_TYPE_UID(Name, Uid, ...) Uid

#define BEAM_REGISTER_TYPE(z, n, q)                                            \
  registry->template add<BEAM_GET_TYPE_NAME q>(BEAM_GET_TYPE_UID q);

#define BEAM_REGISTER_TYPES_(Name, TypeList)                                   \
  template<Beam::IsSender S>                                                   \
  void Name(::Beam::Out< ::Beam::TypeRegistry<S>> registry) {                  \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_TYPE, BOOST_PP_EMPTY, TypeList);      \
  }

#define BEAM_REGISTER_TYPES(Name, ...)                                         \
  BEAM_REGISTER_TYPES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),      \
    (__VA_ARGS__)))

namespace Beam {
namespace Details {
  struct NullShuttle {
    void shuttle(IsShuttle auto&, unsigned int) {}
  };
}

  /**
   * Stores a registry of polymorphic types capable of serialization.
   * @tparam S The type of Sender.
   */
  template<typename S>
  class TypeRegistry {
    public:

      /** Specifies the Sender's type. */
      using Sender = S;

      /** Specifies the Receiver's type. */
      using Receiver = inverse_t<S>;

      /** The TypeEntries stored by this registry. */
      using TypeEntry = Beam::TypeEntry<Sender>;

      /** Constructs a TypeRegistry. */
      TypeRegistry();

      TypeRegistry(const TypeRegistry& registry);

      /**
       * Returns the name for a specified type.
       * @param type The RTTI of the type to get the name for.
       * @return The name of the specified <i>type</i>.
       */
      const std::string& get_type_name(const std::type_index& type) const;

      /**
       * Returns the RTTI for a specified type name.
       * @param name The name of the type to get the RTTI for.
       * @return The RTTI of the specified <i>name</i>.
       */
      std::type_index get_type_index(const std::string& name) const;

      /** Returns the TypeEntry for a specified type. */
      template<typename T>
      const TypeEntry& get_entry() const;

      /** Returns the TypeEntry for a given value. */
      template<typename T>
      const TypeEntry& get_entry(const T& value) const;

      /**
       * Returns the TypeEntry with a given name.
       * @param name The name of the TypeEntry.
       * @return The TypeEntry with the specified <i>name</i>.
       */
      const TypeEntry& get_entry(const std::string& name) const;

      /**
       * Registers a type.
       * @param type The RTTI of the type to register.
       * @param name The name of the type to register.
       */
      void add(std::type_index type, const std::string& name);

      /**
       * Registers a type.
       * @tparam T The type to register.
       * @param name The name of the type to register.
       */
      template<typename T>
      void add(const std::string& name);

      /**
       * Adds all types from an existing registry.
       * @param registry The TypeRegistry to add.
       */
      void add(const TypeRegistry& registry);

    private:
      using TypeEntryIterator =
        typename std::unordered_map<std::type_index, TypeEntry>::iterator;
      std::unordered_map<std::type_index, TypeEntry> m_types;
      std::unordered_map<std::string, TypeEntryIterator> m_type_names;
      std::unordered_map<std::type_index, std::string> m_type_index_names;
      std::unordered_map<std::string, std::type_index> m_type_indexes;

      template<typename T>
      static void send(Sender& sender, const void* value, unsigned int version);
      template<typename T>
      static void receive(
        Receiver& receiver, void* value, unsigned int version);
  };

  template<typename S>
  TypeRegistry<S>::TypeRegistry() {
    add<Details::NullShuttle>("__null");
  }

  template<typename S>
  TypeRegistry<S>::TypeRegistry(const TypeRegistry& registry) {
    add(registry);
  }

  template<typename S>
  const std::string& TypeRegistry<S>::get_type_name(
      const std::type_index& type) const {
    auto i = m_type_index_names.find(type);
    if(i == m_type_index_names.end()) {
      boost::throw_with_location(TypeNotFoundException(type.name()));
    }
    return i->second;
  }

  template<typename S>
  std::type_index TypeRegistry<S>::get_type_index(
      const std::string& name) const {
    auto i = m_type_indexes.find(name);
    if(i == m_type_indexes.end()) {
      boost::throw_with_location(TypeNotFoundException(name));
    }
    return i->second;
  }

  template<typename S>
  template<typename T>
  const TypeEntry<typename TypeRegistry<S>::Sender>&
      TypeRegistry<S>::get_entry() const {
    auto type = std::type_index(typeid(T));
    auto i = m_types.find(type);
    if(i == m_types.end()) {
      boost::throw_with_location(TypeNotFoundException(type.name()));
    }
    return i->second;
  }

  template<typename S>
  template<typename T>
  const TypeEntry<typename TypeRegistry<S>::Sender>&
      TypeRegistry<S>::get_entry(const T& value) const {
    auto type = std::type_index(typeid(value));
    auto i = m_types.find(type);
    if(i == m_types.end()) {
      boost::throw_with_location(TypeNotFoundException(type.name()));
    }
    return i->second;
  }

  template<typename S>
  const TypeEntry<typename TypeRegistry<S>::Sender>&
      TypeRegistry<S>::get_entry(const std::string& name) const {
    auto i = m_type_names.find(name);
    if(i == m_type_names.end()) {
      boost::throw_with_location(TypeNotFoundException(name));
    }
    return i->second->second;
  }

  template<typename S>
  void TypeRegistry<S>::add(std::type_index type, const std::string& name) {
    m_type_index_names.insert_or_assign(type, name);
    m_type_indexes.insert_or_assign(name, type);
  }

  template<typename S>
  template<typename T>
  void TypeRegistry<S>::add(const std::string& name) {
    auto builder = static_cast<T* (*)()>(&DataShuttle::make_new<T>);
    auto sender = &send<T>;
    auto receiver = &receive<T>;
    auto type = std::type_index(typeid(T));
    auto entry = TypeEntry(
      type, name, std::move(builder), std::move(sender), std::move(receiver));
    auto i = m_types.insert(std::pair(type, std::move(entry)));
    if(i.second) {
      m_type_names.insert(std::pair(name, i.first));
    }
  }

  template<typename S>
  void TypeRegistry<S>::add(const TypeRegistry& registry) {
    for(auto& type_entry : registry.m_type_names) {
      auto entry = TypeEntry(type_entry.second->second);
      auto type = entry.get_type();
      auto i = m_types.insert(std::pair(type, std::move(entry)));
      if(i.second) {
        m_type_names.insert(std::pair(type_entry.first, i.first));
      }
    }
  }

  template<typename S>
  template<typename T>
  void TypeRegistry<S>::send(
      Sender& sender, const void* value, unsigned int version) {
    Send<T>()(sender, *static_cast<const T*>(value), version);
  }

  template<typename S>
  template<typename T>
  void TypeRegistry<S>::receive(
      Receiver& receiver, void* value, unsigned int version) {
    Receive<T>()(receiver, *static_cast<T*>(value), version);
  }
}

#endif
