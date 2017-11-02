#ifndef BEAM_FROM_PYTHON_ABSTRACT_QUEUE_HPP
#define BEAM_FROM_PYTHON_ABSTRACT_QUEUE_HPP
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/FromPythonQueueReader.hpp"
#include "Beam/Python/FromPythonQueueWriter.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam {

  /*! \class FromPythonAbstractQueue
      \brief Wraps an AbstractQueue of Python objects to a AbstractQueue of type
             T.
      \tparam T The data to store in the Queue.
   */
  template<typename T>
  class FromPythonAbstractQueue : public AbstractQueue<T> {
    public:
      using Reader = typename AbstractQueue<T>::Reader;
      using Writer = typename AbstractQueue<T>::Writer;
      using Target = typename Reader::Target;
      using Source = typename Writer::Source;

      //! Constructs a default FromPythonAbstractQueue.
      FromPythonAbstractQueue();

      //! Constructs a FromPythonAbstractQueue.
      /*!
        \param queue The AbstractQueue to wrap.
      */
      FromPythonAbstractQueue(
        std::shared_ptr<AbstractQueue<boost::python::object>> queue);

      virtual ~FromPythonAbstractQueue() override final;

      //! Returns the AbstractQueue being wrapped.
      const std::shared_ptr<
        AbstractQueue<boost::python::object>>& GetQueue() const;

      virtual bool IsEmpty() const override final;

      virtual Target Top() const override final;

      virtual void Pop() override final;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

    protected:
      virtual bool IsAvailable() const override final;

    private:
      std::shared_ptr<AbstractQueue<boost::python::object>> m_queue;
      std::shared_ptr<FromPythonQueueReader<Target>> m_reader;
      std::shared_ptr<FromPythonQueueWriter<Source>> m_writer;
  };

  //! Makes a FromPythonAbstractQueue.
  /*!
    \param queue The AbstractQueue to wrap.
  */
  template<typename T>
  auto MakeFromPythonAbstractQueue(
      std::shared_ptr<AbstractQueue<boost::python::object>> queue) {
    return std::make_shared<FromPythonAbstractQueue<T>>(std::move(queue));
  }

  template<typename T>
  FromPythonAbstractQueue<T>::FromPythonAbstractQueue()
      : m_queue{std::make_shared<Queue<boost::python::object>>()},
        m_reader{MakeFromPythonQueueReader<Target>(m_queue)},
        m_writer{MakeFromPythonQueueWriter<Source>(m_queue)} {}

  template<typename T>
  FromPythonAbstractQueue<T>::FromPythonAbstractQueue(
      std::shared_ptr<AbstractQueue<boost::python::object>> queue)
      : m_queue{std::move(queue)},
        m_reader{MakeFromPythonQueueReader<Target>(m_queue)},
        m_writer{MakeFromPythonQueueWriter<Source>(m_queue)} {}

  template<typename T>
  FromPythonAbstractQueue<T>::~FromPythonAbstractQueue() {
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    m_queue.reset();
  }

  template<typename T>
  const std::shared_ptr<AbstractQueue<boost::python::object>>&
      FromPythonAbstractQueue<T>::GetQueue() const {
    return m_queue;
  }

  template<typename T>
  bool FromPythonAbstractQueue<T>::IsEmpty() const {
    return m_reader->IsEmpty();
  }

  template<typename T>
  typename FromPythonAbstractQueue<T>::Target
      FromPythonAbstractQueue<T>::Top() const {
    return m_reader->Top();
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Pop() {
    m_reader->Pop();
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Push(const Source& value) {
    m_writer->Push(value);
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Push(Source&& value) {
    m_writer->Push(std::move(value));
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Break(const std::exception_ptr& e) {
    m_queue->Break(e);
  }

  template<typename T>
  bool FromPythonAbstractQueue<T>::IsAvailable() const {
    return QueueReader<T>::IsAvailable(*m_reader);
  }
}

#endif
