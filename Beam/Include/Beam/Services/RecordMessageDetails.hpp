#ifndef BEAM_RECORD_MESSAGE_DETAILS_HPP
#define BEAM_RECORD_MESSAGE_DETAILS_HPP
#include <functional>
#include <tuple>
#include <utility>
#include <vector>
#include <boost/pfr.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Services/Message.hpp"
#include "Beam/Services/ReportServiceException.hpp"
#include "Beam/Services/ServiceSlot.hpp"

namespace Beam::Details {
  template<typename, typename>
  struct message_to_function;

  template<typename F, typename... Args>
  struct message_to_function<F, std::tuple<Args...>> {
    using type = std::function<void (F&, Args...)>;
  };

  template<typename M>
  struct record_message_slot_type {
    using type =
      typename message_to_function<typename M::ServiceProtocolClient&,
        decltype(boost::pfr::structure_to_tuple(
          std::declval<typename M::Record>()))>::type;
  };

  template<typename M>
  using record_message_slot_t = typename record_message_slot_type<M>::type;

  template<typename M>
  class RecordMessageSlot : public ServiceSlot<M> {
    public:
      using RecordMessage = M;
      using PreHook = typename ServiceSlot<RecordMessage>::PreHook;
      using Slot = record_message_slot_t<RecordMessage>;

      template<typename SF>
      explicit RecordMessageSlot(SF&& slot);

      void invoke(Ref<typename RecordMessage::ServiceProtocolClient> client,
        const typename RecordMessage::Record& record) const;
      void add_pre_hook(const PreHook& hook) override;

    private:
      std::vector<PreHook> m_pre_hooks;
      Slot m_slot;
  };

  template<typename M>
  template<typename SF>
  RecordMessageSlot<M>::RecordMessageSlot(SF&& slot)
    : m_slot(std::forward<SF>(slot)) {}

  template<typename M>
  void RecordMessageSlot<M>::add_pre_hook(const PreHook& hook) {
    m_pre_hooks.push_back(hook);
  }

  template<typename M>
  void RecordMessageSlot<M>::invoke(
      Ref<typename RecordMessage::ServiceProtocolClient> client,
      const typename RecordMessage::Record& record) const {
    auto operation = std::string_view("pre_hook");
    try {
      for(auto& pre_hook : m_pre_hooks) {
        pre_hook(*client.get());
      }
      operation = "handle_message";
      std::apply([&] (const auto&... args) {
        m_slot(*client.get(), args...);
      }, boost::pfr::structure_tie(record));
    } catch(const EndOfFileException&) {
    } catch(...) {
      report_service_exception(
        *client, operation, typeid(RecordMessage), std::nullopt);
    }
  }
}

#endif
