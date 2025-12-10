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
   * @tparam T The type of AbstactQueue to trampoline.
   */
  template<typename T>
  struct TrampolineAbstractQueue final : T {
    using Source = typename T::Source;
    using Target = typename T::Target;
    using T::T;

    Source pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(Source, T, "pop", pop);
    }

    boost::optional<Source> try_pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(
        boost::optional<Source>, T, "try_pop", try_pop);
    }

    void push(Target&& value) override {
      push(value);
    }

    void push(const Target& value) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "push", push, value);
    }

    void close(const std::exception_ptr& e) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "close", close, e);
    }
  };

  /**
   * Wraps an AbstractQueue of Python objects to a AbstractQueue of type T.
   * @tparam T The data to store in the Queue.
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
      explicit FromPythonAbstractQueue(
        std::shared_ptr<AbstractQueue<pybind11::object>> queue);

      ~FromPythonAbstractQueue() override;

      /** Returns the AbstractQueue being wrapped. */
      const std::shared_ptr<AbstractQueue<pybind11::object>>& get_queue() const;

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void push(const Target& value) override;
      void push(Target&& value) override;
      void close(const std::exception_ptr& e) override;
      using AbstractQueue<T>::close;

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
  auto make_from_python_abstract_queue(
      std::shared_ptr<AbstractQueue<pybind11::object>> queue) {
    return std::make_shared<FromPythonAbstractQueue<T>>(std::move(queue));
  }

  template<typename T>
  FromPythonAbstractQueue<T>::FromPythonAbstractQueue()
    : m_queue(std::make_shared<Queue<pybind11::object>>()),
      m_reader(make_from_python_queue_reader<Source>(m_queue)),
      m_writer(make_from_python_queue_writer<Target>(m_queue)) {}

  template<typename T>
  FromPythonAbstractQueue<T>::FromPythonAbstractQueue(
    std::shared_ptr<AbstractQueue<pybind11::object>> queue)
    : m_queue(std::move(queue)),
      m_reader(make_from_python_queue_reader<Source>(m_queue)),
      m_writer(make_from_python_queue_writer<Target>(m_queue)) {}

  template<typename T>
  FromPythonAbstractQueue<T>::~FromPythonAbstractQueue() {
    auto lock = pybind11::gil_scoped_acquire();
    m_queue.reset();
  }

  template<typename T>
  const std::shared_ptr<AbstractQueue<pybind11::object>>&
      FromPythonAbstractQueue<T>::get_queue() const {
    return m_queue;
  }

  template<typename T>
  typename FromPythonAbstractQueue<T>::Source
      FromPythonAbstractQueue<T>::pop() {
    return m_reader->pop();
  }

  template<typename T>
  boost::optional<typename FromPythonAbstractQueue<T>::Source>
      FromPythonAbstractQueue<T>::try_pop() {
    return m_reader->try_pop();
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::push(const Target& value) {
    m_writer->push(value);
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::push(Target&& value) {
    m_writer->push(std::move(value));
  }

  template<typename T>
  void FromPythonAbstractQueue<T>::close(const std::exception_ptr& e) {
    m_queue->close(e);
  }
}

#endif
