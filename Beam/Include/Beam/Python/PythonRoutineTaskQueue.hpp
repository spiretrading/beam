#ifndef BEAM_PYTHONROUTINETASKQUEUE_HPP
#define BEAM_PYTHONROUTINETASKQUEUE_HPP
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/python/object.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Python/PythonQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonRoutineTaskQueue
      \brief Runs Python tasks within a Routine.
   */
  class PythonRoutineTaskQueue : public QueueWriter<boost::python::object> {
    public:

      //! The type being pushed.
      using Source = QueueWriter<boost::python::object>::Source;

      //! Constructs a PythonRoutineTaskQueue.
      PythonRoutineTaskQueue();

      virtual ~PythonRoutineTaskQueue();

      boost::python::object GetSlot(boost::python::object slot);

      void Wait();

      virtual void Push(const Source& value);

      virtual void Push(Source&& value);

      virtual void Break();

      virtual void Break(const std::exception_ptr& e);

    private:
      class Converter : public PythonQueueWriter {
        public:
          Converter(std::shared_ptr<Queue<boost::python::object>> queue,
            boost::python::object slot);

          virtual ~Converter();

          virtual void Push(const Source& value);

          virtual void Push(Source&& value);

          virtual void Break(const std::exception_ptr& e);

          using PythonQueueWriter::Break;
        private:
          std::shared_ptr<Queue<boost::python::object>> m_queue;
          boost::optional<boost::python::object> m_slot;
      };
      mutable boost::mutex m_mutex;
      bool m_isBroken;
      std::shared_ptr<Queue<boost::python::object>> m_queue;
      std::vector<std::shared_ptr<PythonQueueWriter>> m_queues;
      Routines::RoutineHandler m_routine;
  };

  inline PythonRoutineTaskQueue::Converter::Converter(
      std::shared_ptr<Queue<boost::python::object>> queue,
      boost::python::object slot)
      : m_queue{std::move(queue)},
        m_slot{std::move(slot)} {}

  inline PythonRoutineTaskQueue::Converter::~Converter() {
    Break();
  }

  inline void PythonRoutineTaskQueue::Converter::Push(const Source& value) {
    auto slot = m_slot;
    std::function<void ()> callable =
      [=] {
        if(slot != boost::none) {
          (*slot)(value);
        }
      };
    using signature = boost::mpl::vector<void>;
    m_queue->Push(boost::python::make_function(callable,
      boost::python::default_call_policies(), signature()));
  }

  inline void PythonRoutineTaskQueue::Converter::Push(Source&& value) {
    Push(value);
  }

  inline void PythonRoutineTaskQueue::Converter::Break(
      const std::exception_ptr& e) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    m_slot = boost::none;
  }

  inline PythonRoutineTaskQueue::PythonRoutineTaskQueue()
      : m_isBroken(false),
        m_queue(std::make_shared<Queue<boost::python::object>>()) {
    m_routine = Routines::Spawn(
      [=] {
        try {
          GilLock gil;
          while(true) {
            m_queue->Wait();
            boost::lock_guard<GilLock> lock{gil};
            auto callable = m_queue->Top();
            m_queue->Pop();
            try {
              callable();
            } catch(const boost::python::error_already_set&) {
              PrintError();
              return;
            }
          }
        } catch(const PipeBrokenException&) {
          return;
        } catch(const std::exception&) {
          std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        }
      });
  }

  inline PythonRoutineTaskQueue::~PythonRoutineTaskQueue() {
    Break();
    if(Routines::GetCurrentRoutine().GetId() != m_routine.GetId()) {
      GilRelease gil;
      boost::lock_guard<GilRelease> lock{gil};
      m_routine.Wait();
    }
  }

  inline boost::python::object PythonRoutineTaskQueue::GetSlot(
      boost::python::object slot) {
    std::shared_ptr<PythonQueueWriter> converter;
    {
      GilRelease gil;
      boost::lock_guard<GilRelease> lock{gil};
      converter = std::make_shared<Converter>(m_queue, slot);
      bool isBroken;
      {
        boost::lock_guard<boost::mutex> lock{m_mutex};
        m_queues.push_back(converter);
        isBroken = m_isBroken;
      }
      if(isBroken) {
        converter->Break();
      }
    }
    return boost::python::object{converter};
  }

  inline void PythonRoutineTaskQueue::Wait() {
    m_routine.Wait();
  }

  inline void PythonRoutineTaskQueue::Push(const boost::python::object& value) {
    m_queue->Push(value);
  }

  inline void PythonRoutineTaskQueue::Push(boost::python::object&& value) {
    Push(value);
  }

  inline void PythonRoutineTaskQueue::Break(
      const std::exception_ptr& exception) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    std::vector<std::shared_ptr<PythonQueueWriter>> queues;
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      m_isBroken = true;
      queues = m_queues;
    }
    for(auto& queue : queues) {
      queue->Break();
    }
    m_queue->Break(exception);
  }

  inline void PythonRoutineTaskQueue::Break() {
    QueueWriter<boost::python::object>::Break();
  }
}
}

#endif
