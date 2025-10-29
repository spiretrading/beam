#ifndef BEAM_TYPEENTRY_HPP
#define BEAM_TYPEENTRY_HPP
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace Beam {
  template<typename S>
  class TypeRegistry;

  /**
   * Stores serialization meta-data for a polymorphic type.
   * @tparam S The type of Sender.
   */
  template<IsSender S>
  class TypeEntry {
    public:

      /** Specifies the Sender's type. */
      using Sender = S;

      /** Specifies the Receiver's type. */
      using Receiver = inverse_t<Sender>;

      /** Returns the type's RTTI. */
      std::type_index get_type() const;

      /** Returns the type's name. */
      const std::string& get_name() const;

      /**
       * Allocates and constructs an instance of this type.
       * @return A newly constructed instance of <i>T</i>.
       */
      void* make() const;

      /**
       * Sends an instance of this type.
       * @param sender The Sender to use.
       * @param value The value to send.
       * @param version The version of the <i>value</i> to send.
       */
      void send(Sender& sender, const void* value, unsigned int version) const;

      /**
       * Receives an instance of this type.
       * @param receive The Receiver to use.
       * @param value The value to receive.
       * @param version The version of the <i>value</i> to receive.
       */
      void receive(Receiver& receiver, void* value, unsigned int version) const;

    private:
      friend class TypeRegistry<Sender>;
      using SendFunction =
        std::function<void (Sender&, const void*, unsigned int)>;
      using ReceiveFunction =
        std::function<void (Receiver&, void*, unsigned int)>;
      using Factory = std::function<void* ()>;
      std::type_index m_type;
      std::string m_name;
      Factory m_builder;
      SendFunction m_sender;
      ReceiveFunction m_receiver;

      template<typename NF, typename BF, typename SF, typename RF>
      TypeEntry(std::type_index type, NF&& name, BF&& builder, SF&& sender,
        RF&& receiver);
  };

  template<IsSender S>
  std::type_index TypeEntry<S>::get_type() const {
    return m_type;
  }

  template<IsSender S>
  const std::string& TypeEntry<S>::get_name() const {
    return m_name;
  }

  template<IsSender S>
  void* TypeEntry<S>::make() const {
    return m_builder();
  }

  template<IsSender S>
  void TypeEntry<S>::send(
      Sender& sender, const void* value, unsigned int version) const {
    m_sender(sender, value, version);
  }

  template<IsSender S>
  void TypeEntry<S>::receive(
      Receiver& receiver, void* value, unsigned int version) const {
    m_receiver(receiver, value, version);
  }

  template<IsSender S>
  template<typename NF, typename BF, typename SF, typename RF>
  TypeEntry<S>::TypeEntry(
    std::type_index type, NF&& name, BF&& builder, SF&& sender, RF&& receiver)
    : m_type(type),
      m_name(std::forward<NF>(name)),
      m_builder(std::forward<BF>(builder)),
      m_sender(std::forward<SF>(sender)),
      m_receiver(std::forward<RF>(receiver)) {}
}

#endif
