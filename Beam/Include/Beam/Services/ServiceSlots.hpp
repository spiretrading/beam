#ifndef BEAM_SERVICE_SLOTS_HPP
#define BEAM_SERVICE_SLOTS_HPP
#include <memory>
#include <unordered_map>
#include "Beam/Serialization/TypeNotFoundException.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlot.hpp"

namespace Beam::Services {

  /**
   * Stores the slots to call/dispatch to when receiving messages.
   * @param <C> The type of ServiceProtocolClient receiving the messages to
   *        dispatch on.
   */
  template<typename C>
  class ServiceSlots {
    public:

      /** The type of ServiceProtocolClient receiving Service Messages. */
      using ServiceProtocolClient = C;

      /** Constructs a ServiceSlots. */
      ServiceSlots();

      /** Moves a ServiceSlots. */
      ServiceSlots(ServiceSlots&& slots);

      /** Returns the TypeRegistry containing all service types. */
      Serialization::TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender>& GetRegistry();

      /** Returns the TypeRegistry containing all service types. */
      const Serialization::TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender>&
        GetRegistry() const;

      /**
       * Returns the slot associated with a Message.
       * @param message The Message whose slot is to be returned.
       * @return The slot associated with the type of <i>message</i> or nullptr
       *         iff no such slot exists.
       */
      BaseServiceSlot<ServiceProtocolClient>* Find(
        const Message<ServiceProtocolClient>& message) const;

      /**
       * Adds a slot to be associated with a Message.
       * @param <MessageType> The type of Message to associate the slot with.
       * @param slot The slot to associate with the Message.
       */
      template<typename MessageType>
      void Add(std::unique_ptr<ServiceSlot<MessageType>>&& slot);

      /**
       * Adds all slots stored in another ServiceSlots instance.
       * @param slots The ServiceSlots to acquire.
       */
      void Add(ServiceSlots&& slots);

      /**
       * Adds a slot to be associated with a Message.
       * @param <MessageType> The type of Message to associate the slot with.
       * @param slot The slot to associate with the Message.
       */
      template<typename MessageType, typename SlotForward>
      void Add(SlotForward&& slot);

      /**
       * Applies a function to all ServiceSlots.
       * @param f The function to apply.
       */
      template<typename F>
      void Apply(F&& f);

      /** Moves a ServiceSlots. */
      ServiceSlots& operator =(ServiceSlots&& slots);

    private:
      Serialization::TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender> m_registry;
      std::unordered_map<std::string,
        std::unique_ptr<BaseServiceSlot<ServiceProtocolClient>>> m_slots;

      ServiceSlots(const ServiceSlots&) = delete;
      ServiceSlots& operator =(const ServiceSlots&) = delete;
  };

  template<typename C>
  ServiceSlots<C>::ServiceSlots() {
    m_registry.template Register<ServiceRequestException>(
      "Beam.Services.ServiceRequestException");
    m_registry.template Register<HeartbeatMessage<ServiceProtocolClient>>(
      "Beam.Services.HeartbeatMessage");
  }

  template<typename C>
  ServiceSlots<C>::ServiceSlots(ServiceSlots&& slots)
    : m_registry(std::move(slots.m_registry)),
      m_slots(std::move(slots.m_slots)) {}

  template<typename C>
  Serialization::TypeRegistry<
      typename ServiceSlots<C>::ServiceProtocolClient::MessageProtocol::Sender>&
      ServiceSlots<C>::GetRegistry() {
    return m_registry;
  }

  template<typename C>
  const Serialization::TypeRegistry<
      typename ServiceSlots<C>::ServiceProtocolClient::MessageProtocol::Sender>&
      ServiceSlots<C>::GetRegistry() const {
    return m_registry;
  }

  template<typename C>
  BaseServiceSlot<typename ServiceSlots<C>::ServiceProtocolClient>*
      ServiceSlots<C>::Find(
      const Message<ServiceProtocolClient>& message) const {
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

  template<typename C>
  template<typename MessageType>
  void ServiceSlots<C>::Add(std::unique_ptr<ServiceSlot<MessageType>>&& slot) {
    auto& entry = m_registry.template GetEntry<MessageType>();
    m_slots.insert(std::make_pair(entry.GetName(), std::move(slot)));
  }

  template<typename C>
  void ServiceSlots<C>::Add(ServiceSlots&& slots) {
    for(auto& slot : slots.m_slots) {
      m_slots.insert(std::pair(slot.first, std::move(slot.second)));
    }
    m_registry.Add(slots.m_registry);
    slots.m_slots.clear();
  }

  template<typename C>
  template<typename MessageType, typename SlotForward>
  void ServiceSlots<C>::Add(SlotForward&& slot) {
    return Add<ServiceSlot<MessageType>>(
      std::make_unique<typename MessageType::Slot>(
      std::forward<SlotForward>(slot)));
  }

  template<typename C>
  template<typename F>
  void ServiceSlots<C>::Apply(F&& f) {
    for(auto& slot : m_slots) {
      f(slot.first, *slot.second);
    }
  }

  template<typename C>
  ServiceSlots<C>& ServiceSlots<C>::operator =(ServiceSlots&& slots) {
    m_registry = std::move(slots.m_registry);
    m_slots = std::move(slots.m_slots);
    return *this;
  }
}

#endif
