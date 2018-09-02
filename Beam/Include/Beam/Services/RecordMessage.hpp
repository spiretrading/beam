#ifndef BEAM_RECORDMESSAGE_HPP
#define BEAM_RECORDMESSAGE_HPP
#include <utility>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/empty.hpp>
#include <boost/preprocessor/enum_params.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include "Beam/Pointers/UniquePtr.hpp"
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
    BEAM_GET_MESSAGE_NAME q, ServiceProtocolClientType>>(                      \
    BEAM_GET_MESSAGE_UID q);

#define BEAM_DEFINE_MESSAGES_(Name, MessageList)                               \
  BOOST_PP_LIST_FOR_EACH(BEAM_APPLY_MESSAGE, BOOST_PP_EMPTY, MessageList)      \
                                                                               \
  template<typename ServiceProtocolClientType>                                 \
  void Register##Name(::Beam::Out< ::Beam::Services::ServiceSlots<             \
      ServiceProtocolClientType>> slots) {                                     \
    BOOST_PP_LIST_FOR_EACH(BEAM_REGISTER_MESSAGE, BOOST_PP_EMPTY, MessageList) \
  }

#define BEAM_DEFINE_MESSAGES(Name, ...)                                        \
  BEAM_DEFINE_MESSAGES_(Name, BOOST_PP_TUPLE_TO_LIST(PP_NARG(__VA_ARGS__),     \
    (__VA_ARGS__)))

namespace Beam {
namespace Services {

  /*! \class RecordMessage
      \brief Sends a Record type as a Message.
      \tparam RecordType The contents of the Message.
      \tparam ServiceProtocolClientType The type of ServiceProtocolClient
              interpreting this Message.
   */
  template<typename RecordType, typename ServiceProtocolClientType>
  class RecordMessage : public Message<ServiceProtocolClientType> {
    public:

      //! The contents of this Message.
      using Record = RecordType;

      //! The type of ServiceProtocolClient sending/receiving this Message.
      using ServiceProtocolClient = ServiceProtocolClientType;

      //! The type of the slot called when this Message is received.
      using Slot = Details::RecordMessageSlot<RecordMessage>;

      //! Constructs an empty RecordMessage.
      /*!
        \param args The parameters to pass to the record.
      */
      template<typename... Args>
      RecordMessage(Args&&... args);

      //! Returns the Record.
      const Record& GetRecord() const;

      virtual void EmitSignal(BaseServiceSlot<ServiceProtocolClient>* slot,
        Ref<ServiceProtocolClient> protocol) const;

    private:
      friend struct Serialization::DataShuttle;
      Record m_record;

      RecordMessage(Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  //! Adds a slot to be associated with a RecordMessage.
  /*!
    \tparam ServiceProtocolClientType The type of ServiceProtocolClient
            receiving the RecordMessage.
    \param slot The slot handling the RecordMessage.
  */
  template<typename Message, typename ServiceProtocolClientType, typename Slot>
  void AddMessageSlot(Out<ServiceSlots<ServiceProtocolClientType>> serviceSlots,
      Slot&& slot) {
    using RecordMessage =
      Services::RecordMessage<Message, ServiceProtocolClientType>;
    std::unique_ptr<ServiceSlot<RecordMessage>> serviceSlot = std::make_unique<
      Details::RecordMessageSlot<RecordMessage>>(std::forward<Slot>(slot));
    serviceSlots->Add(std::move(serviceSlot));
  }

  //! Sends a message to a ServiceProtocolClient.
  /*!
    \param client The ServiceProtocolClient to send the message to.
    \param args The data to send to the <i>client</i>.
  */
  template<typename RecordType, typename ServiceProtocolClient,
    typename... Args>
  void SendRecordMessage(ServiceProtocolClient& client, const Args&... args) {
    RecordMessage<RecordType, ServiceProtocolClient> message(args...);
    client.Send(message);
  }

  //! Sends a message to a list of ServiceProtocolClients.
  /*!
    \param clients The list of ServiceProtocolClients to send the message to.
    \param args The data to send to the <i>clients</i>.
  */
  template<typename RecordType, typename ServiceProtocolClient,
    typename... Args>
  void BroadcastRecordMessage(
      const std::vector<ServiceProtocolClient*>& clients, const Args&... args) {
    if(clients.empty()) {
      return;
    }

    // TODO: Forward arguments, results in ICE on MSVC 2013.
    RecordMessage<RecordType, ServiceProtocolClient> message(args...);
    if(clients.size() == 1) {
      clients.front()->Send(message);
    } else {
      typename ServiceProtocolClient::MessageProtocol::Channel::Writer::Buffer
        buffer;
      clients.front()->Encode(message, Store(buffer));
      for(auto& client : clients) {
        client->Send(buffer);
      }
    }
  }

  template<typename RecordType, typename ServiceProtocolClientType>
  template<typename... Args>
  RecordMessage<RecordType, ServiceProtocolClientType>::RecordMessage(
      Args&&... args)
      : m_record(std::forward<Args>(args)...) {}

  template<typename RecordType, typename ServiceProtocolClientType>
  const typename RecordMessage<RecordType, ServiceProtocolClientType>::Record&
      RecordMessage<RecordType, ServiceProtocolClientType>::GetRecord() const {
    return m_record;
  }

  template<typename RecordType, typename ServiceProtocolClientType>
  void RecordMessage<RecordType, ServiceProtocolClientType>::EmitSignal(
      BaseServiceSlot<ServiceProtocolClient>* slot,
      Ref<ServiceProtocolClient> protocol) const {
    static_cast<Slot*>(slot)->Invoke(Ref(protocol), m_record);
  }

  template<typename RecordType, typename ServiceProtocolClientType>
  RecordMessage<RecordType, ServiceProtocolClientType>::RecordMessage(
      Serialization::ReceiveBuilder) {}

  template<typename RecordType, typename ServiceProtocolClientType>
  template<typename Shuttler>
  void RecordMessage<RecordType, ServiceProtocolClientType>::Shuttle(
      Shuttler& shuttle, unsigned int version) {
    m_record.Shuttle(shuttle, version);
  }
}
}

namespace Beam {
namespace Serialization {
  template<typename RecordType, typename ServiceProtocolClientType>
  struct IsDefaultConstructable<Services::RecordMessage<
    RecordType, ServiceProtocolClientType>> : std::false_type {};
}
}

#endif
