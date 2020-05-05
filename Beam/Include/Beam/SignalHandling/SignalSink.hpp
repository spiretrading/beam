#ifndef BEAM_SIGNALSINK_HPP
#define BEAM_SIGNALSINK_HPP
#include <deque>
#include <vector>
#include <type_traits>
#include <boost/any.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/at.hpp>
#include <boost/noncopyable.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/SignalHandling/SignalHandling.hpp"
#include "Beam/Threading/TimeoutException.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam {
namespace SignalHandling {
namespace Details {
  template<bool Enabled>
  struct CopyParameter {};

  template<>
  struct CopyParameter<true> {
    template<typename T>
    void operator ()(std::vector<boost::any>& parameters, const T& value) {
      parameters.push_back(&value);
    }
  };

  template<>
  struct CopyParameter<false> {
    template<typename T>
    void operator ()(std::vector<boost::any>& parameters, const T& value) {
      parameters.push_back(value);
    }
  };
}

  /*! \class SignalSink
      \brief Stores signals and their parameters.
   */
  class SignalSink : private boost::noncopyable {
    public:

      /*! \struct SignalEntry
          \brief Stores the contents of a signal.
       */
      struct SignalEntry {

        //! The type of signal.
        const std::type_info* m_type;

        //! Stores the signal's parameters.
        std::vector<boost::any> m_parameters;

        //! Constructs a SignalEntry.
        /*!
          \param type The type of signal.
          \param parameters The list of parameters from the signal.
        */
        SignalEntry(const std::type_info& type,
          const std::vector<boost::any>& parameters);
      };

      //! Constructs a SignalSink.
      SignalSink();

      //! Returns the next signal.
      /*!
        \param timeout The amount of time to wait for the next signal.
        \return The next signal queued.
      */
      SignalEntry GetNextSignal(boost::posix_time::time_duration timeout);

      //! Returns a slot compatible with this signal handler.
      /*!
        \tparam SlotType A function type of the form R (P0, P1, ...)
        \return A slot compatible with this signal handler.
      */
      template<typename SlotType>
      std::function<typename GetSignature<SlotType>::type> GetSlot();

    private:
      boost::mutex m_mutex;
      boost::condition_variable m_signalEmptyCondition;
      std::deque<SignalEntry> m_signalEntries;

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
        static SlotType Invoke(SignalSink* sink);                              \
      };                                                                       \
                                                                               \
      template<typename SlotType, typename ParameterTypes>                     \
      void Slot(BOOST_PP_REPEAT(n, BEAM_DECLARE_PARAMETER, BOOST_PP_EMPTY));

      #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SIGNAL_HANDLING_PARAMETER_COUNT)
      #include BOOST_PP_LOCAL_ITERATE()
      #undef BEAM_DECLARE_PARAMETER
  };

  inline SignalSink::SignalEntry::SignalEntry(const std::type_info& type,
      const std::vector<boost::any>& parameters)
      : m_type(&type),
        m_parameters(parameters) {}

  inline SignalSink::SignalSink() {}

  inline SignalSink::SignalEntry SignalSink::GetNextSignal(
      boost::posix_time::time_duration timeout) {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    while(m_signalEntries.empty()) {
      if(timeout == boost::posix_time::pos_infin) {
        m_signalEmptyCondition.wait(lock);
      } else if(!m_signalEmptyCondition.timed_wait(lock, timeout)) {
        BOOST_THROW_EXCEPTION(Threading::TimeoutException());
      }
    }
    SignalEntry entry = m_signalEntries.front();
    m_signalEntries.pop_front();
    return entry;
  }

  template<typename SlotType>
  std::function<typename GetSignature<SlotType>::type> SignalSink::GetSlot() {
    return GetSlotImplementation<std::function<
      typename GetSignature<SlotType>::type>>::Invoke(this);
  }

  #define BEAM_DECLARE_PARAMETER(z, n, q)                                      \
    BOOST_PP_COMMA_IF(n) typename boost::mpl::at_c<ParameterTypes, n>::type a##n

  #define CALLBACK_PLACEHOLDERS(z, n, a)                                       \
    BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(std::placeholders::_, BOOST_PP_INC(n))

  #define PUSH_BACK_PARAMETERS(z, n, a)                                        \
    Details::CopyParameter<std::is_abstract<typename std::remove_reference<    \
      typename boost::mpl::at_c<ParameterTypes, n>::type>::type>::value>()(    \
      parameters, a ## n);

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename SlotType>                                                  \
  SlotType SignalSink::GetSlotImplementation<SlotType, n>::Invoke(             \
      SignalSink* sink) {                                                      \
    using ParameterTypes = typename boost::function_types::parameter_types<                   \
      typename GetSignature<SlotType>::type>::type;             \
    return std::bind(static_cast<void (SignalSink::*)(                         \
      BOOST_PP_REPEAT(n, BEAM_DECLARE_PARAMETER, BOOST_PP_EMPTY))>(            \
      &SignalSink::Slot<SlotType, ParameterTypes>), sink BOOST_PP_COMMA_IF(n)  \
      BOOST_PP_REPEAT(n, CALLBACK_PLACEHOLDERS, BOOST_PP_EMPTY));              \
  }                                                                            \
                                                                               \
  template<typename SlotType, typename ParameterTypes>                         \
  void SignalSink::Slot(BOOST_PP_REPEAT(n, BEAM_DECLARE_PARAMETER,             \
      BOOST_PP_EMPTY)) {                                                       \
    boost::lock_guard<boost::mutex> lock(m_mutex);                             \
    std::vector<boost::any> parameters;                                        \
    BOOST_PP_REPEAT(n, PUSH_BACK_PARAMETERS, a);                               \
    m_signalEntries.push_back(SignalEntry(typeid(SlotType), parameters));      \
    if(m_signalEntries.size() == 1) {                                          \
      m_signalEmptyCondition.notify_all();                                     \
    }                                                                          \
  }

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_SIGNAL_HANDLING_PARAMETER_COUNT)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef PUSH_BACK_PARAMETERS
  #undef CALLBACK_PLACEHOLDERS
  #undef BEAM_DECLARE_PARAMETER
}
}

#endif
