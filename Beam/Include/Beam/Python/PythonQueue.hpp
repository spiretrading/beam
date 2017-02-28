#ifndef BEAM_PYTHONQUEUE_HPP
#define BEAM_PYTHONQUEUE_HPP
#include <boost/thread/locks.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/PythonQueueWriter.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonQueue
      \brief Provides a Queue implementation that can be used within the Python
             interpreter.
   */
  class PythonQueue : public AbstractQueue<boost::python::object>,
      public PythonQueueWriter {
    public:

      //! Constructs a PythonQueue.
      PythonQueue() = default;

      virtual ~PythonQueue();

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
      Queue<boost::python::object> m_queue;
  };

  inline PythonQueue::~PythonQueue() {
    Break();
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    while(!m_queue.IsEmpty()) {
      m_queue.Pop();
    }
  }

  inline bool PythonQueue::IsEmpty() const {
    return m_queue.IsEmpty();
  }

  inline boost::python::object PythonQueue::Top() const {
    if(IsEmpty()) {
      GilRelease gil;
      boost::lock_guard<GilRelease> release{gil};
      m_queue.Wait();
    }
    return m_queue.Top();
  }

  inline void PythonQueue::Pop() {
    m_queue.Pop();
  }

  inline void PythonQueue::Push(const boost::python::object& value) {
    m_queue.Push(value);
  }

  inline void PythonQueue::Push(boost::python::object&& value) {
    Push(value);
  }

  inline void PythonQueue::Break(const std::exception_ptr& exception) {
    GilRelease gil;
    boost::lock_guard<GilRelease> release{gil};
    PythonQueueWriter::Break(exception);
    m_queue.Break(exception);
  }

  inline void PythonQueue::Break() {
    AbstractQueue<boost::python::object>::Break();
  }

  inline bool PythonQueue::IsAvailable() const {
    return m_queue.IsAvailable();
  }
}
}

#endif
