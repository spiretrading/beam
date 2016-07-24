#ifndef BEAM_PYTHONTASKQUEUE_HPP
#define BEAM_PYTHONTASKQUEUE_HPP
#include <boost/optional.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/PythonQueueWriter.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/TaskQueue.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonTaskQueue
      \brief Provides a TaskQueue implementation that can be used within the
             Python interpreter.
   */
  class PythonTaskQueue : public AbstractQueue<boost::python::object>,
      public PythonQueueWriter {
    public:

      //! Constructs a PythonTaskQueue.
      PythonTaskQueue() = default;

      boost::python::object GetSlot(boost::python::object slot);

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
      TaskQueue m_queue;
  };

  inline boost::python::object PythonTaskQueue::GetSlot(
      boost::python::object slot) {
    auto queueWriter = std::static_pointer_cast<
      QueueWriter<boost::python::object>>(
      m_queue.GetSlot<boost::python::object>(
        [=] (const boost::python::object& value) {
          slot(value);
        }));
    return boost::python::object{queueWriter};
  }

  inline bool PythonTaskQueue::IsEmpty() const {
    return m_queue.IsEmpty();
  }

  inline boost::python::object PythonTaskQueue::Top() const {
    if(IsEmpty()) {
      GilRelease gil;
      boost::unique_lock<GilRelease> release{gil};
      m_queue.Wait();
    }
    auto callable = m_queue.Top();
    auto callPolicies = boost::python::default_call_policies();
    using Signature = boost::mpl::vector<void>;
    return boost::python::make_function(callable, callPolicies, Signature());
  }

  inline void PythonTaskQueue::Pop() {
    m_queue.Pop();
  }

  inline void PythonTaskQueue::Push(const boost::python::object& value) {
    m_queue.Push(value);
  }

  inline void PythonTaskQueue::Push(boost::python::object&& value) {
    Push(value);
  }

  inline void PythonTaskQueue::Break(const std::exception_ptr& exception) {
    GilRelease gil;
    boost::lock_guard<GilRelease> release{gil};
    PythonQueueWriter::Break(exception);
    m_queue.Break(exception);
  }

  inline void PythonTaskQueue::Break() {
    AbstractQueue<boost::python::object>::Break();
  }

  inline bool PythonTaskQueue::IsAvailable() const {
    return m_queue.IsAvailable();
  }
}
}

#endif
