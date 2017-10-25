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

  template<typename T>
  void PublisherWith(Publisher<T>& publisher, boost::python::object callable) {
    GilRelease release;
    boost::lock_guard<GilRelease> lock{release};
    publisher.With(
      [&] {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        callable();
      });
  }

  template<typename T, typename SnapshotType>
  boost::python::object GetSnapshot(
      SnapshotPublisher<T, SnapshotType>& publisher) {
    boost::python::object object;
    {
      GilRelease release;
      boost::lock_guard<GilRelease> lock{release};
      publisher.WithSnapshot(
        [&] (auto snapshot) {
          if(snapshot.is_initialized()) {
            GilLock gil;
            boost::lock_guard<GilLock> lock{gil};
            object = boost::python::object{*snapshot};
          }
        });
    }
    return object;
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
      .def("with", &Details::PublisherWith<T>)
      .def("monitor", BlockingFunction(&Details::PublisherMonitor<T>));
  }

  //! Exports a SnapshotPublisher class.
  template<typename T, typename SnapshotType>
  void ExportSnapshotPublisher(const char* name) {
    boost::python::class_<SnapshotPublisher<T, SnapshotType>,
      boost::noncopyable, boost::python::bases<Publisher<T>,
      BaseSnapshotPublisher>>(name, boost::python::no_init)
      .def("get_snapshot", &Details::GetSnapshot<T, SnapshotType>);
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

  //! Exports the PythonTaskQueue class.
  void ExportTaskQueue();
}
}

#endif
