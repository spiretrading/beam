#ifndef BEAM_MOCKTASK_HPP
#define BEAM_MOCKTASK_HPP
#include "Beam/Queues/SequencePublisher.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/TasksTests/TasksTests.hpp"

namespace Beam {
namespace Tasks {
namespace Tests {

  /*! \class MockTaskFactory
      \brief Implements the TaskFactory for the MockTask.
   */
  class MockTaskFactory : public BasicTaskFactory<MockTaskFactory> {
    public:

      //! The type of callback used when a MockTask is created.
      /*!
        \param task The MockTask that was created.
      */
      typedef std::function<void (const std::shared_ptr<MockTask>& task)>
        CreateCallback;

      //! Constructs a MockTaskFactory.
      MockTaskFactory();

      //! Returns the object publishing Tasks that were created by this factory.
      const SequencePublisher<std::weak_ptr<MockTask>>& GetPublisher() const;

      //! Sets the callback used when a MockTask is created.
      void SetCreateCallback(const CreateCallback& createCallback);

      virtual std::shared_ptr<Task> Create();

      virtual void PrepareContinuation(const Task& task);

      using BasicTaskFactory<MockTaskFactory>::DefineProperty;
      using BasicTaskFactory<MockTaskFactory>::FindProperty;
      using VirtualTaskFactory::FindProperty;

    private:
      std::shared_ptr<SequencePublisher<std::weak_ptr<MockTask>>> m_publisher;
      CreateCallback m_createCallback;
  };

  /*! \class MockTask
      \brief Implements a Task for testing purposes.
   */
  class MockTask : public BasicTask {
    public:

      //! Constructs a MockTask.
      /*!
        \param factory The MockTaskFactory that was used to build this instance.
      */
      MockTask(const MockTaskFactory& factory);

      virtual ~MockTask() {}

      //! Returns the MockTaskFactory used to build this instance.
      const MockTaskFactory& GetFactory() const;

      using BasicTask::SetActive;
      using BasicTask::SetTerminal;

    protected:
      virtual void OnExecute();

      virtual void OnCancel();

    private:
      MockTaskFactory m_factory;
  };

  inline MockTask::MockTask(const MockTaskFactory& factory)
      : m_factory(factory) {}

  inline const MockTaskFactory& MockTask::GetFactory() const {
    return m_factory;
  }

  inline void MockTask::OnExecute() {}

  inline void MockTask::OnCancel() {}

  inline MockTaskFactory::MockTaskFactory()
      : m_publisher(std::make_shared<
          SequencePublisher<std::weak_ptr<MockTask>>>()),
        m_createCallback([] (const std::shared_ptr<MockTask>&) {}) {}

  inline const SequencePublisher<std::weak_ptr<MockTask>>& MockTaskFactory::
      GetPublisher() const {
    return *m_publisher;
  }

  inline void MockTaskFactory::SetCreateCallback(
      const CreateCallback& createCallback) {
    m_createCallback = createCallback;
  }

  inline std::shared_ptr<Task> MockTaskFactory::Create() {
    auto task = std::make_shared<MockTask>(*this);
    m_createCallback(task);
    m_publisher->Push(task);
    return task;
  }

  inline void MockTaskFactory::PrepareContinuation(const Task& task) {}
}
}
}

#endif
