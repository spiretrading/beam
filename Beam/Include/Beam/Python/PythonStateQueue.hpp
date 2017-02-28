#ifndef BEAM_PYTHONSTATEQUEUE_HPP
#define BEAM_PYTHONSTATEQUEUE_HPP
#include <boost/thread/locks.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/PythonQueueWriter.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/StateQueue.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonStateQueue
      \brief Provides a StateQueue implementation that can be used within the
             Python interpreter.
   */
  class PythonStateQueue : public AbstractQueue<boost::python::object>,
      public PythonQueueWriter {
    public:

      //! Constructs a PythonStateQueue.
      PythonStateQueue() = default;

      virtual ~PythonStateQueue();

      virtual bool IsEmpty() const;

      virtual boost::python::object Top() const;

      virtual void Pop();

      virtual void Push(const boost::python::object& value);

      virtual void Push(boost::python::object&& value);

      virtual void Break(const std::exception_ptr& exception);

      virtual void Break();

    protected:
      virtual bool IsAvailable() const;

    private:
      StateQueue<boost::python::object> m_queue;
  };

  inline PythonStateQueue::~PythonStateQueue() {
    Break();
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    while(!m_queue.IsEmpty()) {
      m_queue.Pop();
    }
  }

  inline bool PythonStateQueue::IsEmpty() const {
    return m_queue.IsEmpty();
  }

  inline boost::python::object PythonStateQueue::Top() const {
    if(IsEmpty()) {
      GilRelease gil;
      boost::lock_guard<GilRelease> release{gil};
      m_queue.Wait();
    }
    return m_queue.Top();
  }

  inline void PythonStateQueue::Pop() {
    m_queue.Pop();
  }

  inline void PythonStateQueue::Push(const boost::python::object& value) {
    m_queue.Push(value);
  }

  inline void PythonStateQueue::Push(boost::python::object&& value) {
    Push(value);
  }

  inline void PythonStateQueue::Break(const std::exception_ptr& exception) {
    GilRelease gil;
    boost::lock_guard<GilRelease> release{gil};
    PythonQueueWriter::Break(exception);
    m_queue.Break(exception);
  }

  inline void PythonStateQueue::Break() {
    AbstractQueue<boost::python::object>::Break();
  }

  inline bool PythonStateQueue::IsAvailable() const {
    return m_queue.IsAvailable();
  }
}
}

#endif
