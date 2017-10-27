#ifndef BEAM_PYTHON_QUEUES_HPP
#define BEAM_PYTHON_QUEUES_HPP
#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include "Beam/Python/FromPythonQueueReader.hpp"
#include "Beam/Python/FromPythonQueueWriter.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/PythonQueueWriter.hpp"
#include "Beam/Python/ToPythonQueueReader.hpp"
#include "Beam/Python/ToPythonQueueWriter.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queue.hpp"
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

  template<typename T>
  struct QueueReaderWrapper : T, boost::python::wrapper<T> {
    using Target = typename T::Target;

    virtual bool IsEmpty() const override final {
      return this->get_override("is_empty")();
    }

    virtual Target Top() const override final {
      return this->get_override("top")();
    }

    virtual void Pop() override final {
      this->get_override("pop")();
    }
  };

  template<typename T>
  struct QueueWriterWrapper : T, boost::python::wrapper<T> {
    using Source = typename T::Source;

    virtual void Push(Source&& value) override final {
      Push(value);
    }

    virtual void Push(const Source& value) override final {
      this->get_override("push")(value);
    }
  };

  template<typename T>
  struct AbstractQueueWrapper : T, boost::python::wrapper<T> {};

  template<typename T>
  struct QueueReaderToPython {
    static PyObject* convert(const std::shared_ptr<QueueReader<T>>& queue) {
      auto pythonQueue =
        [&] {
          if(auto pythonQueue =
              std::dynamic_pointer_cast<FromPythonQueueReader<T>>(queue)) {
            return pythonQueue->GetSource();
          }
          return std::static_pointer_cast<QueueReader<boost::python::object>>(
            std::make_shared<ToPythonQueueReader<T>>(queue));
        }();
      boost::python::object value{pythonQueue};
      boost::python::incref(value.ptr());
      return value.ptr();
    }
  };

  template<typename T>
  struct QueueReaderFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object queue{handle};
      boost::python::extract<std::shared_ptr<
        QueueReader<boost::python::object>>> extractor{queue};
      if(extractor.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object value{handle};
      std::shared_ptr<QueueReader<boost::python::object>> queue =
        boost::python::extract<std::shared_ptr<
        QueueReader<boost::python::object>>>(value);
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<QueueReader<T>>>*>(data)->storage.bytes;
      if(auto wrapperQueue = std::dynamic_pointer_cast<
          ToPythonQueueReader<T>>(queue)) {
        new(storage) std::shared_ptr<QueueReader<T>>{wrapperQueue->GetSource()};
      } else {
        new(storage) std::shared_ptr<QueueReader<T>>{
          std::make_shared<FromPythonQueueReader<T>>(queue)};
      }
      data->convertible = storage;
    }
  };

  template<typename T>
  struct QueueWriterToPython {
    static PyObject* convert(const std::shared_ptr<QueueWriter<T>>& queue) {
      auto pythonQueue =
        [&] {
          if(auto pythonQueue =
              std::dynamic_pointer_cast<FromPythonQueueWriter<T>>(queue)) {
            return pythonQueue->GetTarget();
          }
          return std::static_pointer_cast<QueueWriter<boost::python::object>>(
            std::make_shared<ToPythonQueueWriter<T>>(queue));
        }();
      boost::python::object value{pythonQueue};
      boost::python::incref(value.ptr());
      return value.ptr();
    }
  };

  template<typename T>
  struct QueueWriterFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object queue{handle};
      boost::python::extract<std::shared_ptr<
        QueueWriter<boost::python::object>>> extractor{queue};
      if(extractor.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object value{handle};
      std::shared_ptr<QueueWriter<boost::python::object>> queue =
        boost::python::extract<std::shared_ptr<
        QueueWriter<boost::python::object>>>(value);
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<QueueWriter<T>>>*>(data)->storage.bytes;
      if(auto wrapperQueue = std::dynamic_pointer_cast<
          ToPythonQueueWriter<T>>(queue)) {
        new(storage) std::shared_ptr<QueueWriter<T>>{wrapperQueue->GetTarget()};
      } else {
        new(storage) std::shared_ptr<QueueWriter<T>>{
          std::make_shared<FromPythonQueueWriter<T>>(queue)};
      }
      data->convertible = storage;
    }
  };
}

  //! Exports the AbstractQueue class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportAbstractQueue(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Details::AbstractQueueWrapper<T>,
      std::shared_ptr<Details::AbstractQueueWrapper<T>>, boost::noncopyable,
      boost::python::bases<typename T::Reader, typename T::Writer>>(name,
      boost::python::no_init);
    if(!std::is_same<T, AbstractQueue<boost::python::object>>::value) {
/*
      boost::python::to_python_converter<std::shared_ptr<T>,
        Details::QueueReaderToPython<typename T::Target>>();
      boost::python::converter::registry::push_back(
        &Details::QueueReaderFromPythonConverter<
        typename T::Target>::convertible,
        &Details::QueueReaderFromPythonConverter<typename T::Target>::construct,
        boost::python::type_id<std::shared_ptr<T>>());
*/
    } else {
      boost::python::register_ptr_to_python<std::shared_ptr<T>>();
    }
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::AbstractQueueWrapper<T>>, std::shared_ptr<T>>();
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::AbstractQueueWrapper<T>>,
      std::shared_ptr<typename T::Reader>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<typename T::Reader>>();
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::AbstractQueueWrapper<T>>,
      std::shared_ptr<typename T::Writer>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<typename T::Writer>>();
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::AbstractQueueWrapper<T>>,
      std::shared_ptr<BaseQueue>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<BaseQueue>>();
  }

  //! Exports the BasePublisher class.
  void ExportBasePublisher();

  //! Exports the BaseSnapshotPublisher class.
  void ExportBaseSnapshotPublisher();

  //! Exports the BaseQueue class.
  void ExportBaseQueue();

  //! Exports a Publisher class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportPublisher(const char* name) {
    boost::python::class_<Publisher<T>, boost::noncopyable,
      boost::python::bases<BasePublisher>>(name, boost::python::no_init)
      .def("with", &Details::PublisherWith<T>)
      .def("monitor", BlockingFunction(&Details::PublisherMonitor<T>));
  }

  //! Exports a SnapshotPublisher class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T, typename SnapshotType>
  void ExportSnapshotPublisher(const char* name) {
    boost::python::class_<SnapshotPublisher<T, SnapshotType>,
      boost::noncopyable, boost::python::bases<Publisher<T>,
      BaseSnapshotPublisher>>(name, boost::python::no_init)
      .def("get_snapshot", &Details::GetSnapshot<T, SnapshotType>);
  }

  //! Exports the QueueReader class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportQueueReader(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Details::QueueReaderWrapper<T>,
      std::shared_ptr<Details::QueueReaderWrapper<T>>, boost::noncopyable,
      boost::python::bases<BaseQueue>>(name, boost::python::no_init)
      .def("is_empty", boost::python::pure_virtual(&T::IsEmpty))
      .def("top", boost::python::pure_virtual(&T::Top))
      .def("pop", boost::python::pure_virtual(&T::Pop));
    if(!std::is_same<T, QueueReader<boost::python::object>>::value) {
      boost::python::to_python_converter<std::shared_ptr<T>,
        Details::QueueReaderToPython<typename T::Target>>();
      boost::python::converter::registry::push_back(
        &Details::QueueReaderFromPythonConverter<
        typename T::Target>::convertible,
        &Details::QueueReaderFromPythonConverter<typename T::Target>::construct,
        boost::python::type_id<std::shared_ptr<T>>());
    } else {
      boost::python::register_ptr_to_python<std::shared_ptr<T>>();
    }
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::QueueReaderWrapper<T>>, std::shared_ptr<T>>();
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::QueueReaderWrapper<T>>,
      std::shared_ptr<BaseQueue>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<BaseQueue>>();
  }

  //! Exports the QueueWriter class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportQueueWriter(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Details::QueueWriterWrapper<T>,
      std::shared_ptr<Details::QueueWriterWrapper<T>>, boost::noncopyable,
      boost::python::bases<BaseQueue>>(name, boost::python::no_init)
      .def("push", boost::python::pure_virtual(
      static_cast<void (T::*)(const typename T::Source&)>(&T::Push)));
    if(!std::is_same<T, QueueWriter<boost::python::object>>::value) {
      boost::python::to_python_converter<std::shared_ptr<T>,
        Details::QueueWriterToPython<typename T::Source>>();
      boost::python::converter::registry::push_back(
        &Details::QueueWriterFromPythonConverter<
        typename T::Source>::convertible,
        &Details::QueueWriterFromPythonConverter<typename T::Source>::construct,
        boost::python::type_id<std::shared_ptr<T>>());
    } else {
      boost::python::register_ptr_to_python<std::shared_ptr<T>>();
    }
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::QueueWriterWrapper<T>>, std::shared_ptr<T>>();
    boost::python::implicitly_convertible<
      std::shared_ptr<Details::QueueWriterWrapper<T>>,
      std::shared_ptr<BaseQueue>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<BaseQueue>>();
  }

  //! Exports the Queue class.
  /*!
    \param name The name to assign to the class.
  */
  template<typename T>
  void ExportQueue(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<T, std::shared_ptr<T>, boost::noncopyable,
      boost::python::bases<AbstractQueue<typename T::Source>>>(name,
      boost::python::init<>());
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<QueueReader<typename T::Source>>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<QueueWriter<typename T::Source>>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<AbstractQueue<typename T::Source>>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<BaseQueue>>();
  }

  //! Exports the Queues namespace.
  void ExportQueues();

  //! Exports the PythonRoutineTaskQueue class.
  void ExportRoutineTaskQueue();

  //! Exports the PythonTaskQueue class.
  void ExportTaskQueue();
}
}

#endif
