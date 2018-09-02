#ifndef BEAM_RECORDMESSAGEDETAILS_HPP
#define BEAM_RECORDMESSAGEDETAILS_HPP
#include <iostream>
#include <boost/call_traits.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/vector.hpp>
#include "Beam/Services/Message.hpp"
#include "Beam/Services/ServiceSlot.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Services {
namespace Details {
  template<typename ServiceProtocolClientType, typename RecordType>
  struct GetRecordMessageSlotType {};

  #define PASS_PARAMETER(z, n, a)                                              \
    BOOST_PP_COMMA_IF(n) typename boost::call_traits<A##n>::param_type

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename ServiceProtocolClientType BOOST_PP_COMMA_IF(n)             \
    BOOST_PP_ENUM_PARAMS(n, typename A)>                                       \
  struct GetRecordMessageSlotType<ServiceProtocolClientType,                   \
      boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, A)> > {                       \
    typedef std::function<void (ServiceProtocolClientType& BOOST_PP_COMMA_IF(n)\
      BOOST_PP_REPEAT(n, PASS_PARAMETER, BOOST_PP_EMPTY))> type;               \
  };

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SERVICE_PARAMETERS)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef PASS_PARAMETER

  template<typename SlotType, typename RecordMessageType, int I =
    boost::mpl::size<typename RecordMessageType::Record::TypeList>::value>
  struct InvokeRecordMessageSlot {};

  #define GET_RECORD_MEMBER(z, n, a)                                           \
    BOOST_PP_COMMA_IF(n) record.template Get<n>()

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename SlotType, typename RecordMessageType>                      \
  struct InvokeRecordMessageSlot<SlotType, RecordMessageType, n> {             \
    void operator()(const SlotType& slot,                                      \
        typename RecordMessageType::ServiceProtocolClient&                     \
        serviceProtocolClient, const typename RecordMessageType::Record&       \
        record) const {                                                        \
      slot(serviceProtocolClient BOOST_PP_COMMA_IF(n)                          \
        BOOST_PP_REPEAT(n, GET_RECORD_MEMBER, BOOST_PP_EMPTY));                \
    }                                                                          \
  };

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SERVICE_PARAMETERS)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef GET_RECORD_MEMBER

  template<typename RecordMessageType>
  class RecordMessageSlot : public ServiceSlot<RecordMessageType> {
    public:
      typedef RecordMessageType RecordMessage;
      typedef typename ServiceSlot<RecordMessage>::PreHook PreHook;
      typedef typename GetRecordMessageSlotType<
        typename RecordMessage::ServiceProtocolClient,
        typename RecordMessage::Record::TypeList>::type Slot;

      template<typename SlotForward>
      RecordMessageSlot(SlotForward&& slot);

      virtual void Invoke(
        Ref<typename RecordMessage::ServiceProtocolClient> protocol,
        const typename RecordMessage::Record& record) const;

      virtual void AddPreHook(const PreHook& hook);

    private:
      std::vector<PreHook> m_preHooks;
      Slot m_slot;
  };

  template<typename RecordMessageType>
  template<typename SlotForward>
  RecordMessageSlot<RecordMessageType>::RecordMessageSlot(SlotForward&& slot)
      : m_slot(std::forward<SlotForward>(slot)) {}

  template<typename RecordMessageType>
  void RecordMessageSlot<RecordMessageType>::Invoke(
      Ref<typename RecordMessageType::ServiceProtocolClient> protocol,
      const typename RecordMessageType::Record& record) const {
    try {
      for(const PreHook& preHook : m_preHooks) {
        preHook(*protocol.Get());
      }
      InvokeRecordMessageSlot<Slot, RecordMessageType>()(m_slot,
        *protocol.Get(), record);
    } catch(...) {
      std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      return;
    }
  }

  template<typename RecordMessageType>
  void RecordMessageSlot<RecordMessageType>::AddPreHook(const PreHook& hook) {
    m_preHooks.push_back(hook);
  }
}
}
}

#endif
