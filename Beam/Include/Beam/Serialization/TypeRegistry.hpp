#ifndef BEAM_TYPEREGISTRY_HPP
#define BEAM_TYPEREGISTRY_HPP
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <boost/noncopyable.hpp>
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
  template<typename SenderType>                                                \
  void Name(::Beam::Out< ::Beam::Serialization::TypeRegistry<                  \
      SenderType>> registry) {                                                 \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_TYPE, BOOST_PP_EMPTY, TypeList);      \
  }

#define BEAM_REGISTER_TYPES(Name, ...)                                         \
  BEAM_REGISTER_TYPES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),      \
    (__VA_ARGS__)))

namespace Beam {
namespace Serialization {
namespace Details {
  struct NullShuttle {
    template<typename Shuttler>
    void Shuttle(Shuttler&, unsigned int version) {}
  };
}

  /*! \class TypeRegistry
      \brief Stores a registry of polymorphic types capable of serialization.
      \tparam SenderType The type of Sender.
   */
  template<typename SenderType>
  class TypeRegistry : private boost::noncopyable {
    public:

      //! Specifies the Sender's type.
      using Sender = SenderType;

      //! Specifies the Receiver's type.
      using Receiver = typename Inverse<SenderType>::type;

      //! The TypeEntries stored by this registry.
      using TypeEntry = Serialization::TypeEntry<Sender>;

      //! Constructs a TypeRegistry.
      TypeRegistry();

      //! Returns the TypeEntry for a specified type.
      template<typename T>
      const TypeEntry& GetEntry() const;

      //! Returns the TypeEntry for a given value.
      template<typename T>
      const TypeEntry& GetEntry(const T& value) const;

      //! Returns the TypeEntry with a given name.
      /*!
        \param name The name of the TypeEntry.
        \return The TypeEntry with the specified <i>name</i>.
      */
      const TypeEntry& GetEntry(const std::string& name) const;

      //! Registers a type.
      /*!
        \tparam T The type to register.
        \param name The name of the type to register.
      */
      template<typename T>
      void Register(const std::string& name);

      //! Acquires all types in another TypeRegistry.
      /*!
        \param registry The registry whose types are to be acquired.
      */
      void Acquire(TypeRegistry&& registry);

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

  template<typename SenderType>
  TypeRegistry<SenderType>::TypeRegistry() {
    Register<Details::NullShuttle>("__null");
  }

  template<typename SenderType>
  template<typename T>
  const TypeEntry<SenderType>& TypeRegistry<SenderType>::GetEntry() const {
    std::type_index type{typeid(T)};
    auto typeIterator = m_types.find(type);
    if(typeIterator == m_types.end()) {
      throw TypeNotFoundException{type.name()};
    }
    return typeIterator->second;
  }

  template<typename SenderType>
  template<typename T>
  const TypeEntry<SenderType>& TypeRegistry<SenderType>::GetEntry(
      const T& value) const {
    std::type_index type{typeid(*(&const_cast<T&>(value)))};
    auto typeIterator = m_types.find(type);
    if(typeIterator == m_types.end()) {
      throw TypeNotFoundException{type.name()};
    }
    return typeIterator->second;
  }

  template<typename SenderType>
  const TypeEntry<SenderType>& TypeRegistry<SenderType>::GetEntry(
      const std::string& name) const {
    auto typeIterator = m_typeNames.find(name);
    if(typeIterator == m_typeNames.end()) {
      BOOST_THROW_EXCEPTION(TypeNotFoundException{name});
    }
    return typeIterator->second->second;
  }

  template<typename SenderType>
  template<typename T>
  void TypeRegistry<SenderType>::Register(const std::string& name) {
    typename TypeEntry::BuildFunction builder =
      static_cast<T* (*)()>(&DataShuttle::Builder<T>);
    typename TypeEntry::SendFunction sender = &Send<T>;
    typename TypeEntry::ReceiveFunction receiver = &Receive<T>;
    std::type_index type{typeid(T)};
    TypeEntry entry(type, name, std::move(builder), std::move(sender),
      std::move(receiver));
    auto insertResult = m_types.insert(std::make_pair(type, std::move(entry)));
    if(insertResult.second) {
      m_typeNames.insert(std::make_pair(name, insertResult.first));
    }
  }

  template<typename SenderType>
  void TypeRegistry<SenderType>::Acquire(TypeRegistry&& registry) {
    for(auto& typeEntry : registry.m_typeNames) {
      TypeEntry entry{std::move(typeEntry.second->second)};
      std::type_index type{entry.GetType()};
      auto insertResult = m_types.insert(
        std::make_pair(type, std::move(entry)));
      if(insertResult.second) {
        m_typeNames.insert(std::make_pair(typeEntry.first, insertResult.first));
      }
    }
  }

  template<typename SenderType>
  template<typename T>
  void TypeRegistry<SenderType>::Send(Sender& sender, void* value,
      unsigned int version) {
    Serialization::Send<T>()(sender, *static_cast<T*>(value), version);
  }

  template<typename SenderType>
  template<typename T>
  void TypeRegistry<SenderType>::Receive(Receiver& receiver, void* value,
      unsigned int version) {
    Serialization::Receive<T>()(receiver, *static_cast<T*>(value), version);
  }
}
}

#endif
