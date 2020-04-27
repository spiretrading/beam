#ifndef BEAM_SERVICESLOTS_HPP
#define BEAM_SERVICESLOTS_HPP
#include <memory>
#include <unordered_map>
#include "Beam/Serialization/TypeNotFoundException.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlot.hpp"

namespace Beam {
namespace Services {

  /*! \class ServiceSlots
      \brief Stores the slots to call/dispatch to when receiving messages.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClient
              receiving the messages to dispatch on.
   */
  template<typename ServiceProtocolClientType>
  class ServiceSlots {
    public:

      //! The type of ServiceProtocolClient receiving Service Messages.
      using ServiceProtocolClient = ServiceProtocolClientType;

      //! Constructs a ServiceSlots.
      ServiceSlots();

      //! Returns the TypeRegistry containing all service types.
      Serialization::TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender>& GetRegistry();

      //! Returns the TypeRegistry containing all service types.
      const Serialization::TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender>&
        GetRegistry() const;

      //! Returns the slot associated with a Message.
      /*!
        \param message The Message whose slot is to be returned.
        \return The slot associated with the type of <i>message</i> or nullptr
                iff no such slot exists.
      */
      BaseServiceSlot<ServiceProtocolClient>* Find(
        const Message<ServiceProtocolClient>& message);

      //! Adds a slot to be associated with a Message.
      /*!
        \tparam MessageType The type of Message to associate the slot with.
        \param slot The slot to associate with the Message.
      */
      template<typename MessageType>
      void Add(std::unique_ptr<ServiceSlot<MessageType>>&& slot);

      //! Adds a slot to be associated with a Message.
      /*!
        \tparam MessageType The type of Message to associate the slot with.
        \param slot The slot to associate with the Message.
      */
      template<typename MessageType, typename SlotForward>
      void Add(SlotForward&& slot);

      //! Applies a function to all ServiceSlots.
      /*!
        \param f The function to apply.
      */
      template<typename F>
      void Apply(F f);

      //! Acquires all slots stored in another ServiceSlots instance.
      /*!
        \param slots The ServiceSlots to acquire.
      */
      void Acquire(ServiceSlots&& slots);

    private:
      Serialization::TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender> m_registry;
      std::unordered_map<std::string,
        std::unique_ptr<BaseServiceSlot<ServiceProtocolClient>>> m_slots;
  };

  template<typename ServiceProtocolClientType>
  ServiceSlots<ServiceProtocolClientType>::ServiceSlots() {
    m_registry.template Register<ServiceRequestException>(
      "Beam.Services.ServiceRequestException");
    m_registry.template Register<HeartbeatMessage<ServiceProtocolClient>>(
      "Beam.Services.HeartbeatMessage");
  }

  template<typename ServiceProtocolClientType>
  Serialization::TypeRegistry<typename ServiceSlots<
      ServiceProtocolClientType>::ServiceProtocolClient::
      MessageProtocol::Sender>&
      ServiceSlots<ServiceProtocolClientType>::GetRegistry() {
    return m_registry;
  }

  template<typename ServiceProtocolClientType>
  const Serialization::TypeRegistry<typename ServiceSlots<
      ServiceProtocolClientType>::ServiceProtocolClient::
      MessageProtocol::Sender>&
      ServiceSlots<ServiceProtocolClientType>::GetRegistry() const {
    return m_registry;
  }

  template<typename ServiceProtocolClientType>
  BaseServiceSlot<typename ServiceSlots<ServiceProtocolClientType>::
      ServiceProtocolClient>* ServiceSlots<ServiceProtocolClientType>::Find(
      const Message<ServiceProtocolClient>& message) {
    try {
      auto& entry = m_registry.GetEntry(message);
      auto slotIterator = m_slots.find(entry.GetName());
      if(slotIterator == m_slots.end()) {
        return nullptr;
      }
      return slotIterator->second.get();
    } catch(const Serialization::TypeNotFoundException&) {
      return nullptr;
    }
  }

  template<typename ServiceProtocolClientType>
  template<typename MessageType>
  void ServiceSlots<ServiceProtocolClientType>::Add(
      std::unique_ptr<ServiceSlot<MessageType>>&& slot) {
    auto& entry = m_registry.template GetEntry<MessageType>();
    m_slots.insert(std::make_pair(entry.GetName(), std::move(slot)));
  }

  template<typename ServiceProtocolClientType>
  template<typename MessageType, typename SlotForward>
  void ServiceSlots<ServiceProtocolClientType>::Add(SlotForward&& slot) {
    return Add<ServiceSlot<MessageType>>(
      std::make_unique<typename MessageType::Slot>(
      std::forward<SlotForward>(slot)));
  }

  template<typename ServiceProtocolClientType>
  template<typename F>
  void ServiceSlots<ServiceProtocolClientType>::Apply(F f) {
    for(const auto& slot : m_slots) {
      f(slot.first, *slot.second);
    }
  }

  template<typename ServiceProtocolClientType>
  void ServiceSlots<ServiceProtocolClientType>::Acquire(ServiceSlots&& slots) {
    for(auto& slot : slots.m_slots) {
      m_slots.insert(std::make_pair(slot.first, std::move(slot.second)));
    }
    m_registry.Acquire(std::move(slots.m_registry));
    slots.m_slots.clear();
  }
}
}

#endif
