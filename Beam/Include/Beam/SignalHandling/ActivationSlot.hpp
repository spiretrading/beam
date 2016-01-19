#ifndef BEAM_ACTIVATIONSLOT_HPP
#define BEAM_ACTIVATIONSLOT_HPP
#include <memory>
#include "Beam/SignalHandling/SignalHandling.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \class ActivationSlot
      \brief Wraps a slot so that it only gets called after being explicitly
             enabled.
      \tparam SlotType The type of slot to wrap.
   */
  template<typename SlotType>
  class ActivationSlot {
    public:

      //! The type of slot to wrap.
      using Slot = SlotType;

      //! Constructs an ActivationSlot.
      /*!
        \param slot The slot to wrap.
      */
      ActivationSlot(const Slot& slot);

      //! Enables the wrapped slot to receive callbacks.
      void Enable();

      template<typename... A>
      void operator ()(A&&... args) const;

    private:
      mutable Slot m_slot;
      std::shared_ptr<bool> m_isEnabled;
  };

  template<typename SlotType>
  ActivationSlot<SlotType>::ActivationSlot(const Slot& slot)
      : m_slot(slot),
        m_isEnabled(std::make_shared<bool>(false)) {}

  template<typename SlotType>
  void ActivationSlot<SlotType>::Enable() {
    *m_isEnabled = true;
  }

  template<typename SlotType>
  template<typename... A>
  void ActivationSlot<SlotType>::operator ()(A&&... args) const {
    if(*m_isEnabled) {
      m_slot(std::forward<A>(args)...);
    }
  }
}
}

#endif
