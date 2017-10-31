#ifndef BEAM_TO_PYTHON_ABSTRACT_QUEUE_HPP
#define BEAM_TO_PYTHON_ABSTRACT_QUEUE_HPP
#include "Beam/Python/ToPythonQueueReader.hpp"
#include "Beam/Python/ToPythonQueueWriter.hpp"
#include "Beam/Queues/AbstractQueue.hpp"

namespace Beam {

  /*! \class ToPythonAbstractQueue
      \brief Wraps an AbstractQueue of type T to an AbstractQueue of Python
             objects.
      \tparam T The type of data to store in the Queue.
   */
  template<typename T>
  class ToPythonAbstractQueue : public AbstractQueue<boost::python::object> {
    public:
      using Reader = typename AbstractQueue<boost::python::object>::Reader;
      using Writer = typename AbstractQueue<boost::python::object>::Writer;
      using Target = typename Reader::Target;
      using Source = typename Writer::Source;

      //! The type of data to store in the Queue.
      using Type = T;

      //! Constructs a ToPythonAbstractQueue.
      /*!
        \param queue The AbstractQueue to wrap.
      */
      ToPythonAbstractQueue(std::shared_ptr<AbstractQueue<Type>> queue);

      virtual ~ToPythonAbstractQueue() override final = default;

      //! Returns the AbstractQueue being wrapped.
      const std::shared_ptr<AbstractQueue<Type>>& GetQueue() const;

      virtual bool IsEmpty() const override final;

      virtual Target Top() const override final;

      virtual void Pop() override final;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

    protected:
      virtual bool IsAvailable() const override final;

    private:
      std::shared_ptr<AbstractQueue<Type>> m_queue;
      std::shared_ptr<ToPythonQueueReader<Type>> m_reader;
      std::shared_ptr<ToPythonQueueWriter<Type>> m_writer;
  };

  //! Makes a ToPythonAbstractQueue.
  /*!
    \param queue The AbstractQueue to wrap.
  */
  template<typename T>
  auto MakeToPythonAbstractQueue(std::shared_ptr<AbstractQueue<T>> queue) {
    return std::make_shared<ToPythonAbstractQueue<T>>(std::move(queue));
  }

  template<typename T>
  ToPythonAbstractQueue<T>::ToPythonAbstractQueue(
      std::shared_ptr<AbstractQueue<Type>> queue)
      : m_queue{std::move(queue)},
        m_reader{MakeToPythonQueueReader<T>(std::static_pointer_cast<
          QueueReader<T>>(m_queue))},
        m_writer{MakeToPythonQueueWriter<T>(std::static_pointer_cast<
          QueueWriter<T>>(m_queue))} {}

  template<typename T>
  const std::shared_ptr<AbstractQueue<typename ToPythonAbstractQueue<T>::Type>>&
      ToPythonAbstractQueue<T>::GetQueue() const {
    return m_queue;
  }

  template<typename T>
  bool ToPythonAbstractQueue<T>::IsEmpty() const {
    return m_reader->IsEmpty();
  }

  template<typename T>
  typename ToPythonAbstractQueue<T>::Target
      ToPythonAbstractQueue<T>::Top() const {
    return m_reader->Top();
  }

  template<typename T>
  void ToPythonAbstractQueue<T>::Pop() {
    return m_reader->Pop();
  }

  template<typename T>
  void ToPythonAbstractQueue<T>::Push(const Source& value) {
    m_writer->Push(value);
  }

  template<typename T>
  void ToPythonAbstractQueue<T>::Push(Source&& value) {
    return m_writer->Push(std::move(value));
  }

  template<typename T>
  void ToPythonAbstractQueue<T>::Break(const std::exception_ptr& e) {
    m_queue->Break(e);
  }

  template<typename T>
  bool ToPythonAbstractQueue<T>::IsAvailable() const {
    return QueueReader<boost::python::object>::IsAvailable(*m_reader);
  }
}

#endif
