#ifndef BEAM_RECORD_MESSAGE_HPP
#define BEAM_RECORD_MESSAGE_HPP
#include <utility>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleRecord.hpp"
#include "Beam/Services/RecordMessageDetails.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

#define BEAM_DEFINE_MESSAGE(r, data, message)                                  \
  BEAM_DEFINE_RECORD(BOOST_PP_TUPLE_ELEM(0, message),                          \
    BOOST_PP_TUPLE_ENUM(                                                       \
      BOOST_PP_TUPLE_POP_FRONT(                                                \
          BOOST_PP_TUPLE_POP_FRONT(message))))

#define BEAM_REGISTER_MESSAGE(r, data, message)                                \
  slots->get_registry().template add<::Beam::RecordMessage<                    \
    BOOST_PP_TUPLE_ELEM(0, message), C>>(BOOST_PP_TUPLE_ELEM(1, message));

#define BEAM_DEFINE_MESSAGES(name, ...)                                        \
  BOOST_PP_SEQ_FOR_EACH(                                                       \
    BEAM_DEFINE_MESSAGE, *, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))             \
                                                                               \
  template<typename C>                                                         \
  void BOOST_PP_CAT(register_, name)(                                          \
      ::Beam::Out<::Beam::ServiceSlots<C>> slots) {                            \
    BOOST_PP_SEQ_FOR_EACH(                                                     \
      BEAM_REGISTER_MESSAGE, *, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))         \
  }

namespace Beam {

  /**
   * Sends a Record type as a Message.
   * @tparam R The contents of the Message.
   * @tparam C The type of ServiceProtocolClient interpreting this Message.
   */
  template<typename R, typename C>
  class RecordMessage : public Message<C> {
    public:

      /** The contents of this Message. */
      using Record = R;

      /** The type of ServiceProtocolClient sending/receiving this Message. */
      using ServiceProtocolClient = C;

      /** The type of the slot called when this Message is received. */
      using Slot = Details::RecordMessageSlot<RecordMessage>;

      /**
       * Constructs an empty RecordMessage.
       * @param args The parameters to pass to the record.
       */
      template<typename... Args>
      explicit RecordMessage(Args&&... args);

      /** Returns the Record. */
      const Record& get_record() const;

      void emit(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> client) const override;

    private:
      friend struct Beam::DataShuttle;
      Record m_record;

      RecordMessage() = default;
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  /**
   * Adds a slot to be associated with a RecordMessage.
   * @param service_slots The ServiceSlots to add the slot to.
   * @param slot The slot handling the RecordMessage.
   */
  template<typename Message, typename ServiceProtocolClient, typename Slot>
  void add_message_slot(
      Out<ServiceSlots<ServiceProtocolClient>> service_slots, Slot&& slot) {
    using RecordMessage = Beam::RecordMessage<Message, ServiceProtocolClient>;
    service_slots->add(
      std::make_unique<Details::RecordMessageSlot<RecordMessage>>(
        std::forward<Slot>(slot)));
  }

  /**
   * Sends a message to a ServiceProtocolClient.
   * @param client The ServiceProtocolClient to send the message to.
   * @param args The data to send to the <i>client</i>.
   */
  template<typename R, typename ServiceProtocolClient, typename... Args>
  void send_record_message(ServiceProtocolClient& client, Args&&... args) {
    auto message =
      RecordMessage<R, ServiceProtocolClient>(std::forward<Args>(args)...);
    try {
      client.send(message);
    } catch(const std::exception&) {
      return;
    }
  }

  /**
   * Sends a message to a list of ServiceProtocolClients.
   * @param clients The list of ServiceProtocolClients to send the message to.
   * @param args The data to send to the <i>clients</i>.
   */
  template<typename R, typename ServiceProtocolClient, typename... Args>
  void broadcast_record_message(
      const std::vector<ServiceProtocolClient*>& clients, Args&&... args) {
    if(clients.empty()) {
      return;
    } else if(clients.size() == 1) {
      send_record_message<R>(*clients.front(), std::forward<Args>(args)...);
    } else {
      auto message =
        RecordMessage<R, ServiceProtocolClient>(std::forward<Args>(args)...);
      auto buffer = SharedBuffer();
      clients.front()->encode(message, out(buffer));
      for(auto& client : clients) {
        try {
          client->send(buffer);
        } catch(const std::exception&) {
          continue;
        }
      }
    }
  }

  template<typename R, typename C>
  template<typename... Args>
  RecordMessage<R, C>::RecordMessage(Args&&... args)
    : m_record(std::forward<Args>(args)...) {}

  template<typename R, typename C>
  const typename RecordMessage<R, C>::Record&
      RecordMessage<R, C>::get_record() const {
    return m_record;
  }

  template<typename R, typename C>
  void RecordMessage<R, C>::emit(BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> client) const {
    static_cast<Slot*>(slot)->invoke(Ref(client), m_record);
  }

  template<typename R, typename C>
  template<IsShuttle S>
  void RecordMessage<R, C>::shuttle(S& shuttle, unsigned int version) {
    m_record.shuttle(shuttle, version);
  }
}

#endif
