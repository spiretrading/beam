#ifndef BEAM_SCOPED_SLOT_ADAPTOR_HPP
#define BEAM_SCOPED_SLOT_ADAPTOR_HPP
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {
namespace Details {
  template<typename F>
  struct ScopedSlot {
    std::shared_ptr<Sync<bool, RecursiveMutex>> m_is_alive;
    F m_slot;

    template<typename SF>
    ScopedSlot(
      std::shared_ptr<Sync<bool, RecursiveMutex>> keep_alive, SF&& slot)
      : m_is_alive(std::move(keep_alive)),
        m_slot(std::forward<SF>(slot)) {}

    template<typename... Args>
    decltype(auto) operator ()(Args&&... args) {
      return with(*m_is_alive, [&] (auto is_alive) {
        if(is_alive) {
          return m_slot(std::forward<Args>(args)...);
        } else {
          boost::throw_with_location(std::runtime_error("Slot expired."));
        }
      });
    }
  };
}

  /**
   * Produces slots that only get invoked if the object that constructed it is
   * in scope.
   */
  class ScopedSlotAdaptor {
    public:

      /** Constructs a ScopedSlotAdaptor. */
      ScopedSlotAdaptor();

      ~ScopedSlotAdaptor();

      /**
       * Returns a slot whose scope is tied to this object.
       * @param slot The slot to scope.
       */
      template<typename F>
      Details::ScopedSlot<std::remove_cvref_t<F>> make_slot(F&& slot) const;

    private:
      std::shared_ptr<Sync<bool, RecursiveMutex>> m_is_alive;
  };

  inline ScopedSlotAdaptor::ScopedSlotAdaptor()
    : m_is_alive(std::make_shared<Sync<bool, RecursiveMutex>>(true)) {}

  inline ScopedSlotAdaptor::~ScopedSlotAdaptor() {
    *m_is_alive = false;
  }

  template<typename F>
  Details::ScopedSlot<std::remove_cvref_t<F>> ScopedSlotAdaptor::make_slot(
      F&& slot) const {
    return Details::ScopedSlot<std::remove_cvref_t<F>>(
      m_is_alive, std::forward<F>(slot));
  }
}

#endif
