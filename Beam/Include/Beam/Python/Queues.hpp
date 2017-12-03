#ifndef BEAM_PYTHON_QUEUES_HPP
#define BEAM_PYTHON_QUEUES_HPP
#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include "Beam/Python/FromPythonAbstractQueue.hpp"
#include "Beam/Python/FromPythonQueueReader.hpp"
#include "Beam/Python/FromPythonQueueWriter.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Python/ToPythonAbstractQueue.hpp"
#include "Beam/Python/ToPythonQueueReader.hpp"
#include "Beam/Python/ToPythonQueueWriter.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"

namespace Beam {
namespace Python {
namespace Details {
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
  struct AbstractQueueWrapper : T, boost::python::wrapper<T> {
    using Target = typename T::Target;
    using Source = typename T::Source;

    virtual bool IsEmpty() const override final {
      return this->get_override("is_empty")();
    }

    virtual Target Top() const override final {
      return this->get_override("top")();
    }

    virtual void Pop() override final {
      this->get_override("pop")();
    }

    virtual void Push(Source&& value) override final {
      Push(value);
    }

    virtual void Push(const Source& value) override final {
      this->get_override("push")(value);
    }
  };

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
            MakeToPythonQueueReader(queue));
        }();
      return boost::python::incref(boost::python::object{pythonQueue}.ptr());
    }
  };

  template<typename T>
  struct QueueReaderFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(boost::python::extract<std::shared_ptr<
          QueueReader<boost::python::object>>>{object}.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto queue = boost::python::extract<std::shared_ptr<
        QueueReader<boost::python::object>>>{object}();
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<QueueReader<T>>>*>(data)->storage.bytes;
      if(auto wrapperQueue = std::dynamic_pointer_cast<ToPythonQueueReader<T>>(
          queue)) {
        new(storage) std::shared_ptr<QueueReader<T>>{wrapperQueue->GetSource()};
      } else {
        new(storage) std::shared_ptr<QueueReader<T>>{
          std::make_shared<FromPythonQueueReader<T>>(std::move(queue))};
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
            MakeToPythonQueueWriter(queue));
        }();
      return boost::python::incref(boost::python::object{pythonQueue}.ptr());
    }
  };

  template<typename T>
  struct MultiQueueWriterToPython {
    static PyObject* convert(
        const std::shared_ptr<MultiQueueWriter<T>>& queue) {
      return QueueWriterToPython<T>::convert(
        std::static_pointer_cast<QueueWriter<T>>(queue));
    }
  };

  template<typename T>
  struct QueueWriterFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(boost::python::extract<std::shared_ptr<
          QueueWriter<boost::python::object>>>{object}.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto queue = boost::python::extract<std::shared_ptr<
        QueueWriter<boost::python::object>>>{object}();
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<QueueWriter<T>>>*>(data)->storage.bytes;
      if(auto wrapperQueue = std::dynamic_pointer_cast<ToPythonQueueWriter<T>>(
          queue)) {
        auto target = wrapperQueue->GetTarget();
        if(target != nullptr) {
          new(storage) std::shared_ptr<QueueWriter<T>>{std::move(target)};
        } else {
          new(storage) std::shared_ptr<QueueWriter<T>>{
            MakeFromPythonQueueWriter<T>(std::move(queue))};
        }
      } else {
        new(storage) std::shared_ptr<QueueWriter<T>>{
          MakeFromPythonQueueWriter<T>(std::move(queue))};
      }
      data->convertible = storage;
    }
  };

  template<typename T>
  struct AbstractQueueToPython {
    static PyObject* convert(const std::shared_ptr<AbstractQueue<T>>& queue) {
      auto pythonQueue =
        [&] {
          if(auto pythonQueue =
              std::dynamic_pointer_cast<FromPythonAbstractQueue<T>>(queue)) {
            return pythonQueue->GetQueue();
          }
          return std::static_pointer_cast<AbstractQueue<boost::python::object>>(
            MakeToPythonAbstractQueue(queue));
        }();
      return boost::python::incref(boost::python::object{pythonQueue}.ptr());
    }
  };

  template<typename T>
  struct AbstractQueueFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(boost::python::extract<std::shared_ptr<
          AbstractQueue<boost::python::object>>>{object}.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto queue = boost::python::extract<std::shared_ptr<
        AbstractQueue<boost::python::object>>>{object}();
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        std::shared_ptr<AbstractQueue<T>>>*>(data)->storage.bytes;
      if(auto wrapperQueue = std::dynamic_pointer_cast<
          ToPythonAbstractQueue<T>>(queue)) {
        auto target = wrapperQueue->GetQueue();
        if(target != nullptr) {
          new(storage) std::shared_ptr<AbstractQueue<T>>{std::move(target)};
        } else {
          new(storage) std::shared_ptr<AbstractQueue<T>>{
            MakeFromPythonAbstractQueue<T>(std::move(queue))};
        }
      } else {
        new(storage) std::shared_ptr<AbstractQueue<T>>{
          MakeFromPythonAbstractQueue<T>(std::move(queue))};
      }
      data->convertible = storage;
    }
  };

  template<typename T, typename Enabled = void>
  struct GetCallPolicies;

  template<typename T>
  struct GetCallPolicies<T,
      typename std::enable_if<std::is_pointer<T>::value>::type> {
    auto operator ()() const {
      return boost::python::return_value_policy<
        boost::python::reference_existing_object>();
    }
  };

  template<typename T, typename Enabled>
  struct GetCallPolicies {
    auto operator ()() const {
      return boost::python::default_call_policies();
    }
  };
}

#ifdef _MSC_VER
#define BEAM_DEFINE_PYTHON_QUEUE_LINKER(T)                                     \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Publisher<T>);                       \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::AbstractQueue<T>);                   \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::MultiQueueWriter<T>);                \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::QueueReader<T>);                     \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::QueueWriter<T>);                     \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Queue<T>);                           \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Python::Details::QueueReaderWrapper< \
    Beam::QueueReader<T>>);                                                    \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Python::Details::QueueWriterWrapper< \
    Beam::QueueWriter<T>>);                                                    \
  BEAM_DEFINE_PYTHON_POINTER_LINKER(                                           \
    Beam::Python::Details::AbstractQueueWrapper<Beam::AbstractQueue<T>>);
#else
#define BEAM_DEFINE_PYTHON_QUEUE_LINKER(T)
#endif

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
      boost::python::to_python_converter<std::shared_ptr<T>,
        Details::AbstractQueueToPython<typename T::Target>>();
      boost::python::converter::registry::push_back(
        &Details::AbstractQueueFromPythonConverter<
        typename T::Target>::convertible,
        &Details::AbstractQueueFromPythonConverter<
        typename T::Target>::construct,
        boost::python::type_id<std::shared_ptr<T>>());
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

  //! Exports the MultiQueueWriter class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportMultiQueueWriter(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<T, std::shared_ptr<T>, boost::noncopyable,
      boost::python::bases<QueueWriter<typename T::Source>,
      Publisher<typename T::Source>>>(name, boost::python::init<>());
    if(!std::is_same<T, MultiQueueWriter<boost::python::object>>::value) {
      boost::python::converter::registry::push_back(
        &Details::QueueWriterFromPythonConverter<
        typename T::Source>::convertible,
        &Details::QueueWriterFromPythonConverter<typename T::Source>::construct,
        boost::python::type_id<std::shared_ptr<T>>());
    }
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<QueueWriter<typename T::Source>>>();
  }

  //! Exports a Publisher class.
  /*!
    \param name The name to assign to the type.
  */
  template<typename T>
  void ExportPublisher(const char* name) {
    boost::python::class_<Publisher<T>, boost::noncopyable,
      boost::python::bases<BasePublisher>>(name, boost::python::no_init)
      .def("with", BlockingFunction(&Publisher<T>::With))
      .def("monitor", BlockingFunction(&Publisher<T>::Monitor));
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
      .def("top", boost::python::pure_virtual(&T::Top),
        Details::GetCallPolicies<typename T::Target>{}())
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
      boost::python::init<>())
      .def("is_broken", &T::IsBroken)
      .def("wait", BlockingFunction(static_cast<void (T::*)() const>(&T::Wait)))
      .def("top", BlockingFunction(&T::Top,
        Details::GetCallPolicies<typename T::Source>{}()));
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<QueueReader<typename T::Source>>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<QueueWriter<typename T::Source>>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<AbstractQueue<typename T::Source>>>();
    boost::python::implicitly_convertible<std::shared_ptr<T>,
      std::shared_ptr<BaseQueue>>();
  }

  //! Exports the Queue class.
  /*!
    \param name The name to assign to the class.
  */
  template<>
  inline void ExportQueue<Queue<boost::python::object>>(const char* name) {
    using ExportedType = FromPythonAbstractQueue<boost::python::object>;
    auto typeId = boost::python::type_id<ExportedType>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<ExportedType, std::shared_ptr<ExportedType>,
      boost::noncopyable,
      boost::python::bases<AbstractQueue<boost::python::object>>>(name,
      boost::python::init<>());
    boost::python::implicitly_convertible<std::shared_ptr<ExportedType>,
      std::shared_ptr<QueueReader<boost::python::object>>>();
    boost::python::implicitly_convertible<std::shared_ptr<ExportedType>,
      std::shared_ptr<QueueWriter<boost::python::object>>>();
    boost::python::implicitly_convertible<std::shared_ptr<ExportedType>,
      std::shared_ptr<AbstractQueue<boost::python::object>>>();
    boost::python::implicitly_convertible<std::shared_ptr<ExportedType>,
      std::shared_ptr<BaseQueue>>();
  }

  //! Exports a suite of queue classes.
  /*!
    \param baseName The name of the type to export.
  */
  template<typename T>
  void ExportQueueSuite(const char* baseName) {
    ExportQueueReader<QueueReader<T>>(
      (baseName + std::string{"QueueReader"}).c_str());
    ExportQueueWriter<QueueWriter<T>>(
      (baseName + std::string{"QueueWriter"}).c_str());
    ExportAbstractQueue<AbstractQueue<T>>(
      (baseName + std::string{"AbstractQueue"}).c_str());
    ExportQueue<Queue<T>>((baseName + std::string{"Queue"}).c_str());
    ExportPublisher<T>((baseName + std::string{"Publisher"}).c_str());
    ExportMultiQueueWriter<MultiQueueWriter<T>>(
      (baseName + std::string{"MultiQueueWriter"}).c_str());
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
