#ifndef BEAM_SERVICE_SLOTS_HPP
#define BEAM_SERVICE_SLOTS_HPP
#include <concepts>
#include <memory>
#include <unordered_map>
#include "Beam/Serialization/TypeNotFoundException.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"
#include "Beam/Services/HeartbeatMessage.hpp"
#include "Beam/Services/RequestToken.hpp"
#include "Beam/Services/ServiceSlot.hpp"

namespace Beam {

  /**
   * Stores the slots to call/dispatch to when receiving messages.
   * @tparam C The type of ServiceProtocolClient receiving the messages to
   *        dispatch on.
   */
  template<typename C>
  class ServiceSlots {
    public:

      /** The type of ServiceProtocolClient receiving Service Messages. */
      using ServiceProtocolClient = C;

      /** Constructs a ServiceSlots. */
      ServiceSlots();

      ServiceSlots(ServiceSlots&& slots) = default;

      /** Returns the TypeRegistry containing all service types. */
      TypeRegistry<typename ServiceProtocolClient::MessageProtocol::Sender>&
        get_registry();

      /** Returns the TypeRegistry containing all service types. */
      const TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender>&
          get_registry() const;

      /**
       * Returns the slot associated with a Message.
       * @param message The Message whose slot is to be returned.
       * @return The slot associated with the type of <i>message</i> or nullptr
       *         iff no such slot exists.
       */
      BaseServiceSlot<ServiceProtocolClient>* find(
        const Message<ServiceProtocolClient>& message) const;

      /**
       * Adds a slot to be associated with a Message.
       * @tparam MessageType The type of Message to associate the slot with.
       * @param slot The slot to associate with the Message.
       */
      template<typename Slot>
      void add(std::unique_ptr<Slot> slot);

      /**
       * Adds all slots stored in another ServiceSlots instance.
       * @param slots The ServiceSlots to acquire.
       */
      void add(ServiceSlots&& slots);

      /**
       * Adds a slot to be associated with a Message.
       * @tparam MessageType The type of Message to associate the slot with.
       * @param slot The slot to associate with the Message.
       */
      template<typename MessageType, typename SlotForward>
      void add(SlotForward&& slot);

      /**
       * Applies a function to all ServiceSlots.
       * @param f The function to apply.
       */
      template<std::invocable<const std::string&, BaseServiceSlot<C>&> F>
      void apply(F f);

      ServiceSlots& operator =(ServiceSlots&& slots) = default;

    private:
      TypeRegistry<
        typename ServiceProtocolClient::MessageProtocol::Sender> m_registry;
      std::unordered_map<std::string,
        std::unique_ptr<BaseServiceSlot<ServiceProtocolClient>>> m_slots;

      ServiceSlots(const ServiceSlots&) = delete;
      ServiceSlots& operator =(const ServiceSlots&) = delete;
  };

  template<typename C>
  ServiceSlots<C>::ServiceSlots() {
    m_registry.template add<ServiceRequestException>(
      "Beam.Services.ServiceRequestException");
    m_registry.template add<HeartbeatMessage<ServiceProtocolClient>>(
      "Beam.Services.HeartbeatMessage");
  }

  template<typename C>
  TypeRegistry<
    typename ServiceSlots<C>::ServiceProtocolClient::MessageProtocol::Sender>&
      ServiceSlots<C>::get_registry() {
    return m_registry;
  }

  template<typename C>
  const TypeRegistry<
    typename ServiceSlots<C>::ServiceProtocolClient::MessageProtocol::Sender>&
      ServiceSlots<C>::get_registry() const {
    return m_registry;
  }

  template<typename C>
  BaseServiceSlot<typename ServiceSlots<C>::ServiceProtocolClient>*
    ServiceSlots<C>::find(
      const Message<ServiceProtocolClient>& message) const {
    try {
      auto& entry = m_registry.get_entry(message);
      auto i = m_slots.find(entry.get_name());
      if(i == m_slots.end()) {
        return nullptr;
      }
      return i->second.get();
    } catch(const TypeNotFoundException&) {
      return nullptr;
    }
  }

  template<typename C>
  template<typename Slot>
  void ServiceSlots<C>::add(std::unique_ptr<Slot> slot) {
    auto& entry = m_registry.template get_entry<typename Slot::Message>();
    m_slots.insert(std::pair(entry.get_name(), std::move(slot)));
  }

  template<typename C>
  void ServiceSlots<C>::add(ServiceSlots&& slots) {
    for(auto& slot : slots.m_slots) {
      m_slots.insert(std::pair(slot.first, std::move(slot.second)));
    }
    m_registry.add(slots.m_registry);
    slots.m_slots.clear();
  }

  template<typename C>
  template<typename MessageType, typename SlotForward>
  void ServiceSlots<C>::add(SlotForward&& slot) {
    return add(std::make_unique<typename MessageType::Slot>(
      std::forward<SlotForward>(slot)));
  }

  template<typename C>
  template<std::invocable<const std::string&, BaseServiceSlot<C>&> F>
  void ServiceSlots<C>::apply(F f) {
    for(auto& slot : m_slots) {
      f(slot.first, *slot.second);
    }
  }
}

#endif
