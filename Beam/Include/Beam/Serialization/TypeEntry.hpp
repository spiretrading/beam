#ifndef BEAM_TYPEENTRY_HPP
#define BEAM_TYPEENTRY_HPP
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include "Beam/Serialization/Serialization.hpp"

namespace Beam {
namespace Serialization {

  /*! \class TypeEntry
      \brief Stores serialization meta-data for a polymorphic type.
      \tparam SenderType The type of Sender.
   */
  template<typename SenderType>
  class TypeEntry {
    public:
      static_assert(IsSender<SenderType>::value,
        "SenderType must implement the Sender Concept.");
      static_assert(IsReceiver<typename Inverse<SenderType>::type>::value,
        "Sender's Inverse must implement the Receiver Concept.");

      //! Specifies the Sender's type.
      using Sender = SenderType;

      //! Specifies the Receiver's type.
      using Receiver = typename Inverse<SenderType>::type;

      //! Returns the type's RTTI.
      std::type_index GetType() const;

      //! Returns the type's name.
      const std::string& GetName() const;

      //! Allocates and constructs an instance of this type.
      /*!
        \return A newly built instance of <i>T</i>.
      */
      template<typename T>
      T* Build() const;

      //! Sends an instance of this type.
      /*!
        \tparam T The type to send, it's RTTI must match GetType()'s.
        \param sender The Sender to use.
        \param value The value to send.
        \param version The version of the <i>value</i> to send.
      */
      template<typename T>
      void Send(Sender& sender, T* const& value, unsigned int version) const;

      //! Receives an instance of this type.
      /*!
        \tparam T The type to receive, it's RTTI must match GetType()'s.
        \param receive The Receiver to use.
        \param value The value to receive.
        \param version The version of the <i>value</i> to receive.
      */
      template<typename T>
      void Receive(Receiver& receiver, T* value, unsigned int version) const;

    private:
      template<typename S> friend class TypeRegistry;
      using SendFunction =
        std::function<void(Sender&, void* const, unsigned int)>;
      using ReceiveFunction =
        std::function<void(Receiver&, void*, unsigned int)>;
      using BuildFunction = std::function<void*()>;
      std::type_index m_type;
      std::string m_name;
      BuildFunction m_builder;
      SendFunction m_sender;
      ReceiveFunction m_receiver;

      template<typename NameForward, typename BuilderForward,
        typename SenderForward, typename ReceiverForward>
      TypeEntry(std::type_index type, NameForward&& name,
        BuilderForward&& builder, SenderForward&& sender,
        ReceiverForward&& receiver);
  };

  template<typename SenderType>
  std::type_index TypeEntry<SenderType>::GetType() const {
    return m_type;
  }

  template<typename SenderType>
  const std::string& TypeEntry<SenderType>::GetName() const {
    return m_name;
  }

  template<typename SenderType>
  template<typename T>
  T* TypeEntry<SenderType>::Build() const {
    return static_cast<T*>(m_builder());
  }

  template<typename SenderType>
  template<typename T>
  void TypeEntry<SenderType>::Send(Sender& sender, T* const& value,
      unsigned int version) const {
    m_sender(sender, const_cast<typename std::remove_const<T>::type*>(value),
      version);
  }

  template<typename SenderType>
  template<typename T>
  void TypeEntry<SenderType>::Receive(Receiver& receiver, T* value,
      unsigned int version) const {
    m_receiver(receiver, value, version);
  }

  template<typename SenderType>
  template<typename NameForward, typename BuilderForward,
    typename SenderForward, typename ReceiverForward>
  TypeEntry<SenderType>::TypeEntry(std::type_index type,
      NameForward&& name, BuilderForward&& builder, SenderForward&& sender,
      ReceiverForward&& receiver)
      : m_type(type),
        m_name(std::forward<NameForward>(name)),
        m_builder(std::forward<BuilderForward>(builder)),
        m_sender(std::forward<SenderForward>(sender)),
        m_receiver(std::forward<ReceiverForward>(receiver)) {}
}
}

#endif
