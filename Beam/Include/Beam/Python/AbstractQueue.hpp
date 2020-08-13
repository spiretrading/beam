#ifndef BEAM_PYTHON_ABSTRACT_QUEUE_HPP
#define BEAM_PYTHON_ABSTRACT_QUEUE_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/QueueReader.hpp"
#include "Beam/Python/QueueWriter.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/Queue.hpp"

namespace Beam::Python {

  /**
   * Provides a trampoline template for exporting AbstractQueue classes.
   * @param <T> The type of AbstactQueue to trampoline.
   */
  template<typename T>
  struct TrampolineAbstractQueue final : T {
    using Source = typename T::Source;
    using Target = typename T::Target;
    using T::T;
  };

  /**
   * Wraps an AbstractQueue of Python objects to a AbstractQueue of type T.
   * @param <T> The data to store in the Queue.
   */
  template<typename T>
  class FromPythonAbstractQueue final : public AbstractQueue<T> {
    public:
      using Reader = typename AbstractQueue<T>::Reader;
      using Writer = typename AbstractQueue<T>::Writer;
      using Source = typename Reader::Source;
      using Target = typename Writer::Target;

      /** Constructs a default FromPythonAbstractQueue. */
      FromPythonAbstractQueue();

      /**
       * Constructs a FromPythonAbstractQueue.
       * param queue The AbstractQueue to wrap.
       */
      FromPythonAbstractQueue(
        std::shared_ptr<AbstractQueue<pybind11::object>> queue);

      ~FromPythonAbstractQueue() override;

      //! Returns the AbstractQueue being wrapped.
      const std::shared_ptr<AbstractQueue<pybind11::object>>& GetQueue() const;

      Source Pop() override;

      boost::optional<Source> TryPop() override;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

      using AbstractQueue<T>::Break;

    private:
      std::shared_ptr<AbstractQueue<pybind11::object>> m_queue;
      std::shared_ptr<FromPythonQueueReader<Source>> m_reader;
      std::shared_ptr<FromPythonQueueWriter<Target>> m_writer;
  };

  /**
   * Makes a FromPythonAbstractQueue.
   * @param queue The AbstractQueue to wrap.
   */
  template<typename T>
  auto MakeFromPythonAbstractQueue(
      std::shared_ptr<AbstractQueue<pybind11::object>> queue) {
    return std::make_shared<FromPythonAbstractQueue<T>>(std::move(queue));
  }

  template<typename T>
  FromPythonAbstractQueue<T>::FromPythonAbstractQueue()
    : m_queue(std::make_shared<Queue<pybind11::object>>()),
      m_reader(MakeFromPythonQueueReader<Source>(m_queue)),
      m_writer(MakeFromPythonQueueWriter<Target>(m_queue)) {}

  template<typename T>
  FromPythonAbstractQueue<T>::FromPythonAbstractQueue(
    std::shared_ptr<AbstractQueue<pybind11::object>> queue)
    : m_queue(std::move(queue)),
      m_reader(MakeFromPythonQueueReader<Source>(m_queue)),
      m_writer(MakeFromPythonQueueWriter<Target>(m_queue)) {}

  template<typename T>
  FromPythonAbstractQueue<T>::~FromPythonAbstractQueue() {
    auto lock = GilLock();
    m_queue.reset();
  }

  template<typename T>
  const std::shared_ptr<AbstractQueue<pybind11::object>>&
      FromPythonAbstractQueue<T>::GetQueue() const {
    return m_queue;
  }

  template<typename T>
  typename FromPythonAbstractQueue<T>::Source
      FromPythonAbstractQueue<T>::Pop() {
    return m_reader->Pop();
  }

  template<typename T>
  boost::optional<typename FromPythonAbstractQueue<T>::Source>
      FromPythonAbstractQueue<T>::TryPop() {
    return m_reader->TryPop();
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Push(const Target& value) {
    m_writer->Push(value);
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Push(Target&& value) {
    m_writer->Push(std::move(value));
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::Break(const std::exception_ptr& e) {
    m_queue->Break(e);
  }
}

#endif
