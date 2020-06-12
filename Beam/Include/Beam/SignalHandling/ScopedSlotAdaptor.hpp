#ifndef BEAM_SCOPED_SLOT_ADAPTOR_HPP
#define BEAM_SCOPED_SLOT_ADAPTOR_HPP
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/noncopyable.hpp>
#include "Beam/SignalHandling/SignalHandling.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam::SignalHandling {
namespace Details {
  template<typename F>
  struct ScopedSlot {
    std::shared_ptr<Threading::Sync<bool, Threading::RecursiveMutex>> m_isAlive;
    F m_slot;

    template<typename SlotForward>
    ScopedSlot(std::shared_ptr<
      Threading::Sync<bool, Threading::RecursiveMutex>> keepAlive,
      SlotForward&& slot)
      : m_isAlive(std::move(keepAlive)),
        m_slot(std::forward<SlotForward>(slot)) {}

    template<typename... Args>
    decltype(auto) operator ()(Args&&... args) {
      return Threading::With(*m_isAlive,
        [&] (bool isAlive) {
          if(isAlive) {
            return m_slot(std::forward<Args>(args)...);
          } else {
            throw std::runtime_error("Slot expired.");
          }
        });
    }
  };
}

  /**
   * Produces slots that only get invoked if the object that constructed it
   * is in scope.
   */
  class ScopedSlotAdaptor : private boost::noncopyable {
    public:

      /** Constructs a ScopedSlotAdaptor. */
      ScopedSlotAdaptor();

      ~ScopedSlotAdaptor();

      /**
       * Returns a slot whose scope is tied to this object.
       *
       * @param slot The slot to scopt.
       */
      template<typename F>
      Details::ScopedSlot<std::decay_t<F>> MakeSlot(F&& slot) const;

    private:
      std::shared_ptr<Threading::Sync<bool, Threading::RecursiveMutex>>
        m_isAlive;
  };

  inline ScopedSlotAdaptor::ScopedSlotAdaptor()
    : m_isAlive(std::make_shared<
        Threading::Sync<bool, Threading::RecursiveMutex>>(true)) {}

  inline ScopedSlotAdaptor::~ScopedSlotAdaptor() {
    *m_isAlive = false;
  }

  template<typename F>
  Details::ScopedSlot<std::decay_t<F>> ScopedSlotAdaptor::MakeSlot(
      F&& slot) const {
    return Details::ScopedSlot<std::decay_t<F>>(m_isAlive,
      std::forward<F>(slot));
  }
}

#endif
