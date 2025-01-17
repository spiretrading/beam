#ifndef BEAM_QUEUED_SIGNAL_HANDLER_HPP
#define BEAM_QUEUED_SIGNAL_HANDLER_HPP
#include <mutex>
#include <vector>
#include <boost/call_traits.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/at.hpp>
#include <boost/noncopyable.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include "Beam/SignalHandling/SignalHandling.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam::SignalHandling {

  /** Allows for the delayed handling of signals. */
  class QueuedSignalHandler : private boost::noncopyable {
    public:

      /** Indicates that one or more signals have been queued. */
      using QueuedSlot = std::function<void()>;

      /** Constructs a QueuedSignalHandler. */
      QueuedSignalHandler();

      /**
       * Sets the slot called when signals are queued.
       * @param queuedSlot The slot to call when signals are queued.
       */
      void SetQueuedSlot(const QueuedSlot& queuedSlot);

      /** Dispatches all signals. */
      void HandleSignals();

      /**
       * Queues a task to be performed.
       * @param task The task to perform.
       */
      void QueueTask(const std::function<void ()>& task);

      /**
       * Returns a slot compatible with this signal handler.
       * @param <SlotType> A function type of the form R (P0, P1, ...)
       * @param slot The slot to make compatible with this signal handler.
       * @return A slot compatible with this signal handler.
       */
      template<typename SlotType>
      std::function<typename GetSignature<SlotType>::type> GetSlot(
        const SlotType& slot);

    private:
      std::mutex m_mutex;
      QueuedSlot m_queuedSlot;
      std::vector<std::function<void ()>> m_slots;

      template<typename SlotType, int Arity =
        boost::function_types::function_arity<
        typename GetSignature<SlotType>::type>::value>
      struct GetSlotImplementation {};

      #define BEAM_DECLARE_PARAMETER(z, n, q)                                  \
        BOOST_PP_COMMA_IF(n) typename boost::mpl::at_c<ParameterTypes, n>::type\
        a##n

      #define BOOST_PP_LOCAL_MACRO(n)                                          \
      template<typename SlotType>                                              \
      struct GetSlotImplementation<SlotType, n> {                              \
        static SlotType Invoke(QueuedSignalHandler* handler,                   \
          const SlotType& slot);                                               \
      };                                                                       \
                                                                               \
      template<typename SlotType, typename ParameterTypes>                     \
      void Slot(const SlotType& slot BOOST_PP_COMMA_IF(n)                      \
        BOOST_PP_REPEAT(n, BEAM_DECLARE_PARAMETER, BOOST_PP_EMPTY));

      #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SIGNAL_HANDLING_PARAMETER_COUNT)
      #include BOOST_PP_LOCAL_ITERATE()
      #undef BEAM_DECLARE_PARAMETER
  };

  inline QueuedSignalHandler::QueuedSignalHandler() {}

  inline void QueuedSignalHandler::SetQueuedSlot(const QueuedSlot& queuedSlot) {
    auto lock = std::lock_guard(m_mutex);
    m_queuedSlot = queuedSlot;
  }

  inline void QueuedSignalHandler::HandleSignals() {
    auto slots = std::vector<std::function<void ()>>();
    {
      auto lock = std::lock_guard(m_mutex);
      slots.swap(m_slots);
    }
    for(auto& slot : slots) {
      slot();
    }
  }

  inline void QueuedSignalHandler::QueueTask(
      const std::function<void ()>& task) {
    auto lock = std::lock_guard(m_mutex);
    m_slots.push_back(task);
    if(m_slots.size() == 1 && m_queuedSlot) {
      m_queuedSlot();
    }
  }

  template<typename SlotType>
  std::function<typename GetSignature<SlotType>::type>
      QueuedSignalHandler::GetSlot(const SlotType& slot) {
    return GetSlotImplementation<std::function<
      typename GetSignature<SlotType>::type>>::Invoke(this, slot);
  }

  #define BEAM_DECLARE_PARAMETER(z, n, q)                                      \
    BOOST_PP_COMMA_IF(n) typename boost::mpl::at_c<ParameterTypes, n>::type a##n

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename SlotType>                                                  \
  SlotType QueuedSignalHandler::GetSlotImplementation<SlotType, n>::Invoke(    \
      QueuedSignalHandler* handler, const SlotType& slot) {                    \
    using ParameterTypes = typename boost::function_types::parameter_types<    \
      typename GetSignature<SlotType>::type>::type;                            \
    return std::bind_front(static_cast<void (QueuedSignalHandler::*)(          \
      const SlotType& BOOST_PP_COMMA_IF(n)                                     \
      BOOST_PP_REPEAT(n, BEAM_DECLARE_PARAMETER, BOOST_PP_EMPTY))>(            \
      &QueuedSignalHandler::Slot<SlotType, ParameterTypes>), handler, slot);   \
  }                                                                            \
                                                                               \
  template<typename SlotType, typename ParameterTypes>                         \
  void QueuedSignalHandler::Slot(const SlotType& slot BOOST_PP_COMMA_IF(n)     \
      BOOST_PP_REPEAT(n, BEAM_DECLARE_PARAMETER, BOOST_PP_EMPTY)) {            \
    auto lock = std::lock_guard(m_mutex);                                      \
    m_slots.push_back(std::bind_front(slot BOOST_PP_COMMA_IF(n)                \
      BOOST_PP_ENUM_PARAMS(n, a)));                                            \
    if(m_slots.size() == 1 && m_queuedSlot) {                                  \
      m_queuedSlot();                                                          \
    }                                                                          \
  }

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SIGNAL_HANDLING_PARAMETER_COUNT)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef BEAM_CALLBACK_PLACEHOLDERS
  #undef BEAM_DECLARE_PARAMETER
}

#endif
