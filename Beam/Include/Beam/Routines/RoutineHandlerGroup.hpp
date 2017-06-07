#ifndef BEAM_ROUTINEHANDLERGROUP_HPP
#define BEAM_ROUTINEHANDLERGROUP_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/Routines.hpp"

namespace Beam {
namespace Routines {

  /*! \class RoutineHandlerGroup
      \brief Stores a collection of RoutineHandlers.
   */
  class RoutineHandlerGroup : private boost::noncopyable {
    public:

      //! Constructs an empty RoutineHandlerGroup.
      RoutineHandlerGroup() = default;

      ~RoutineHandlerGroup();

      //! Adds a RoutineHandler to this group.
      /*!
        \param handler The RoutineHandler to add.
      */
      void Add(RoutineHandler&& handler);

      //! Creates a RoutineHandler from an Routine::Id and adds it to this
      //! group.
      /*!
        \param id The Id of the Routine to add.
      */
      void Add(Routine::Id id);

      //! Spawns a Routine and adds it to this group.
      template<typename F>
      void Spawn(F&& f);

      //! Waits for the completion of all Routines in this group.
      void Wait();

    private:
      mutable boost::mutex m_mutex;
      std::vector<RoutineHandler> m_routines;
  };

  inline RoutineHandlerGroup::~RoutineHandlerGroup() {
    Wait();
  }

  inline void RoutineHandlerGroup::Add(RoutineHandler&& handler) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    m_routines.push_back(std::move(handler));
  }

  inline void RoutineHandlerGroup::Add(Routine::Id id) {
    RoutineHandler routine{id};
    Add(std::move(routine));
  }

  template<typename F>
  void RoutineHandlerGroup::Spawn(F&& f) {
    Add(Routines::Spawn(std::forward<F>(f)));
  }

  inline void RoutineHandlerGroup::Wait() {
    std::vector<RoutineHandler> routines;
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      routines.swap(m_routines);
    }
  }
}
}

#endif
