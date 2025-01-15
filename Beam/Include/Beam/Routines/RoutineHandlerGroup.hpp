#ifndef BEAM_ROUTINE_HANDLER_GROUP_HPP
#define BEAM_ROUTINE_HANDLER_GROUP_HPP
#include <mutex>
#include <vector>
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/Routines.hpp"

namespace Beam {
namespace Routines {

  /** Stores a collection of RoutineHandlers. */
  class RoutineHandlerGroup {
    public:

      /** Constructs an empty RoutineHandlerGroup. */
      RoutineHandlerGroup() = default;

      ~RoutineHandlerGroup();

      /**
       * Adds a RoutineHandler to this group.
       * @param handler The RoutineHandler to add.
       */
      void Add(RoutineHandler&& handler);

      /**
       * Creates a RoutineHandler from an Routine::Id and adds it to this group.
       * @param id The Id of the Routine to add.
       */
      void Add(Routine::Id id);

      /** Spawns a Routine and adds it to this group. */
      template<typename F>
      void Spawn(F&& f);

      /** Waits for the completion of all Routines in this group. */
      void Wait();

    private:
      mutable std::mutex m_mutex;
      std::vector<RoutineHandler> m_routines;

      RoutineHandlerGroup(const RoutineHandlerGroup&) = delete;
      RoutineHandlerGroup& operator =(const RoutineHandlerGroup&) = delete;
  };

  inline RoutineHandlerGroup::~RoutineHandlerGroup() {
    Wait();
  }

  inline void RoutineHandlerGroup::Add(RoutineHandler&& handler) {
    auto lock = std::lock_guard(m_mutex);
    m_routines.push_back(std::move(handler));
  }

  inline void RoutineHandlerGroup::Add(Routine::Id id) {
    Add(RoutineHandler(id));
  }

  template<typename F>
  void RoutineHandlerGroup::Spawn(F&& f) {
    Add(Routines::Spawn(std::forward<F>(f)));
  }

  inline void RoutineHandlerGroup::Wait() {
    auto routines = std::vector<RoutineHandler>();
    {
      auto lock = std::lock_guard(m_mutex);
      routines.swap(m_routines);
    }
  }
}
}

#endif
