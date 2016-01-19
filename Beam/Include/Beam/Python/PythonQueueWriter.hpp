#ifndef BEAM_PYTHONQUEUEWRITER_HPP
#define BEAM_PYTHONQUEUEWRITER_HPP
#include <type_traits>
#include <boost/python/object.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonQueueWriter
      \brief Provides an interface for converting between a Python Queue and
             a C++ queue.
   */
  class PythonQueueWriter : public virtual QueueWriter<boost::python::object> {
    public:

      //! The type being pushed.
      using Source = QueueWriter<boost::python::object>::Source;

      //! Constructs a PythonQueueWriter.
      PythonQueueWriter() = default;

      template<typename T>
      typename std::enable_if<!std::is_pointer<T>::value,
        std::shared_ptr<QueueWriter<T>>>::type GetSlot();

      template<typename T>
      typename std::enable_if<std::is_pointer<T>::value,
        std::shared_ptr<QueueWriter<T>>>::type GetSlot();

      virtual void Break(const std::exception_ptr& exception);

      using QueueWriter<boost::python::object>::Break;
    private:
      CallbackQueue m_callbacks;
  };

  template<typename T>
  typename std::enable_if<!std::is_pointer<T>::value,
      std::shared_ptr<QueueWriter<T>>>::type PythonQueueWriter::GetSlot() {
    return m_callbacks.GetSlot<T>(
      [=] (const T& value) {
        GilLock gil;
        boost::lock_guard<GilLock> lock(gil);
        boost::python::object object(value);
        Push(object);
      },
      [=] (const std::exception_ptr& exception) {
        Break(exception);
      });
  }

  template<typename T>
  typename std::enable_if<std::is_pointer<T>::value,
      std::shared_ptr<QueueWriter<T>>>::type PythonQueueWriter::GetSlot() {
    return m_callbacks.GetSlot<T>(
      [=] (const T& value) {
        GilLock gil;
        boost::lock_guard<GilLock> lock(gil);
        boost::python::object object(boost::ref(*value));
        Push(object);
      },
      [=] (const std::exception_ptr& exception) {
        Break(exception);
      });
  }

  inline void PythonQueueWriter::Break(const std::exception_ptr& exception) {
    m_callbacks.Break(exception);
  }
}
}

#endif
