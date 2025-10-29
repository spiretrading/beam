#ifndef BEAM_ROUTINE_HANDLER_GROUP_HPP
#define BEAM_ROUTINE_HANDLER_GROUP_HPP
#include <vector>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {

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
      void add(RoutineHandler&& handler);

      /**
       * Creates a RoutineHandler from an Routine::Id and adds it to this
       * group.
       * @param id The Id of the Routine to add.
       */
      void add(Routine::Id id);

      /** Spawns a Routine and adds it to this group. */
      template<typename F>
      void spawn(F&& f);

      /** Waits for the completion of all Routines in this group. */
      void wait();

    private:
      mutable boost::mutex m_mutex;
      std::vector<RoutineHandler> m_routines;
  };

  inline RoutineHandlerGroup::~RoutineHandlerGroup() {
    wait();
  }

  inline void RoutineHandlerGroup::add(RoutineHandler&& handler) {
    auto lock = boost::lock_guard(m_mutex);
    m_routines.push_back(std::move(handler));
  }

  inline void RoutineHandlerGroup::add(Routine::Id id) {
    add(RoutineHandler(id));
  }

  template<typename F>
  void RoutineHandlerGroup::spawn(F&& f) {
    add(Beam::spawn(std::forward<F>(f)));
  }

  inline void RoutineHandlerGroup::wait() {
    auto routines = std::vector<RoutineHandler>();
    {
      auto lock = boost::lock_guard(m_mutex);
      routines.swap(m_routines);
    }
  }
}

#endif
