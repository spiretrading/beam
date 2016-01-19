#ifndef BEAM_PYTHONQUEUES_HPP
#define BEAM_PYTHONQUEUES_HPP
#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/PythonQueueWriter.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  void PublisherMonitor(Publisher<T>& publisher,
      std::shared_ptr<PythonQueueWriter> monitor) {
    publisher.Monitor(monitor->GetSlot<T>());
  }
}

  //! Exports the AbstractQueue class.
  void ExportAbstractQueue();

  //! Exports the BasePublisher class.
  void ExportBasePublisher();

  //! Exports the BaseSnapshotPublisher class.
  void ExportBaseSnapshotPublisher();

  //! Exports the BaseQueue class.
  void ExportBaseQueue();

  //! Exports a Publisher class.
  template<typename T>
  void ExportPublisher(const char* name) {
    boost::python::class_<Publisher<T>, boost::noncopyable,
      boost::python::bases<BasePublisher>>(name, boost::python::no_init)
      .def("lock", BlockingFunction(&Publisher<T>::Lock))
      .def("unlock", BlockingFunction(&Publisher<T>::Unlock))
      .def("with", &Publisher<T>::With)
      .def("monitor", &Details::PublisherMonitor<T>);
  }

  //! Exports a SnapshotPublisher class.
  template<typename T, typename SnapshotType>
  void ExportSnapshotPublisher(const char* name) {
    boost::python::class_<SnapshotPublisher<T, SnapshotType>,
      boost::noncopyable, boost::python::bases<Publisher<T>,
      BaseSnapshotPublisher>>(name, boost::python::no_init);
  }

  //! Exports the PythonQueueWriter class.
  void ExportPythonQueueWriter();

  //! Exports the PythonQueue class.
  void ExportQueue();

  //! Exports the QueueReader class.
  void ExportQueueReader();

  //! Exports the Queues namespace.
  void ExportQueues();

  //! Exports the QueueWriter class.
  void ExportQueueWriter();

  //! Exports the PythonRoutineTaskQueue class.
  void ExportRoutineTaskQueue();

  //! Exports the PythonStateQueue class.
  void ExportStateQueue();
}
}

#endif
