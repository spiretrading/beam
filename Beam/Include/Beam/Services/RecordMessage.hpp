#ifndef BEAM_RECORD_MESSAGE_HPP
#define BEAM_RECORD_MESSAGE_HPP
#include <utility>
#include <boost/preprocessor/empty.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleRecord.hpp"
#include "Beam/Services/RecordMessageDetails.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Preprocessor.hpp"

#define BEAM_DEFINE_MESSAGE(Name, Uid, ...)                                    \
  BEAM_DEFINE_RECORD(Name, __VA_ARGS__);

#define BEAM_APPLY_MESSAGE(z, n, q) BEAM_DEFINE_MESSAGE q
#define BEAM_GET_MESSAGE_NAME(Name, ...) Name
#define BEAM_GET_MESSAGE_UID(Name, Uid, ...) Uid

#define BEAM_REGISTER_MESSAGE(z, n, q)                                         \
  slots->GetRegistry().template Register< ::Beam::Services::RecordMessage<     \
    BEAM_GET_MESSAGE_NAME q, C>>(BEAM_GET_MESSAGE_UID q);

#define BEAM_DEFINE_MESSAGES_(Name, MessageList)                               \
  BOOST_PP_LIST_FOR_EACH(BEAM_APPLY_MESSAGE, BOOST_PP_EMPTY, MessageList)      \
                                                                               \
  template<typename C>                                                         \
  void Register##Name(::Beam::Out< ::Beam::Services::ServiceSlots<C>> slots) { \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_MESSAGE, BOOST_PP_EMPTY, MessageList) \
  }

#define BEAM_DEFINE_MESSAGES(Name, ...)                                        \
  BEAM_DEFINE_MESSAGES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),     \
    (__VA_ARGS__)))

namespace Beam {
namespace Services {

  /**
   * Sends a Record type as a Message.
   * @param <R> The contents of the Message.
   * @param <C> The type of ServiceProtocolClient interpreting this Message.
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
      RecordMessage(Args&&... args);

      /** Returns the Record. */
      const Record& GetRecord() const;

      void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> protocol) const override;

    private:
      friend struct Serialization::DataShuttle;
      Record m_record;

      RecordMessage(Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  /**
   * Adds a slot to be associated with a RecordMessage.
   * @param <Client> The type of ServiceProtocolClient receiving the
   *        RecordMessage.
   * @param slot The slot handling the RecordMessage.
   */
  template<typename Message, typename ServiceProtocolClient, typename Slot>
  void AddMessageSlot(Out<ServiceSlots<ServiceProtocolClient>> serviceSlots,
      Slot&& slot) {
    using RecordMessage =
      Services::RecordMessage<Message, ServiceProtocolClient>;
    serviceSlots->Add(
      std::make_unique<Details::RecordMessageSlot<RecordMessage>>(
      std::forward<Slot>(slot)));
  }

  /**
   * Sends a message to a ServiceProtocolClient.
   * @param client The ServiceProtocolClient to send the message to.
   * @param args The data to send to the <i>client</i>.
   */
  template<typename R, typename ServiceProtocolClient, typename... Args>
  void SendRecordMessage(ServiceProtocolClient& client, Args&&... args) {
    auto message = RecordMessage<R, ServiceProtocolClient>(
      std::forward<Args>(args)...);
    try {
      client.Send(message);
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
  void BroadcastRecordMessage(
      const std::vector<ServiceProtocolClient*>& clients, Args&&... args) {
    if(clients.empty()) {
      return;
    } else if(clients.size() == 1) {
      SendRecordMessage<R>(*clients.front(), std::forward<Args>(args)...);
    } else {
      auto message = RecordMessage<R, ServiceProtocolClient>(
        std::forward<Args>(args)...);
      auto buffer = typename
        ServiceProtocolClient::MessageProtocol::Channel::Writer::Buffer();
      clients.front()->Encode(message, Store(buffer));
      for(auto& client : clients) {
        try {
          client->Send(buffer);
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
      RecordMessage<R, C>::GetRecord() const {
    return m_record;
  }

  template<typename R, typename C>
  void RecordMessage<R, C>::EmitSignal(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> protocol) const {
    static_cast<Slot*>(slot)->Invoke(Ref(protocol), m_record);
  }

  template<typename R, typename C>
  RecordMessage<R, C>::RecordMessage(Serialization::ReceiveBuilder) {}

  template<typename R, typename C>
  template<typename Shuttler>
  void RecordMessage<R, C>::Shuttle(Shuttler& shuttle, unsigned int version) {
    m_record.Shuttle(shuttle, version);
  }
}

namespace Serialization {
  template<typename R, typename C>
  struct IsDefaultConstructable<Services::RecordMessage<R, C>> :
    std::false_type {};
}
}

#endif
