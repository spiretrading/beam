#ifndef BEAM_TEST_READER_HPP
#define BEAM_TEST_READER_HPP
#include <limits>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/ServicesTests/TestServiceClientOperationQueue.hpp"

namespace Beam::Tests {

  /** A reader that queues its operations for a test to complete. */
  class TestReader {
    public:

      /** Records a call to poll(). */
      struct PollOperation {

        /** Used to return whether data is available. */
        ServiceResult<bool> m_result;
      };

      /** Records a call to read(...). */
      struct ReadOperation {

        /** The maximum number of bytes requested. */
        std::size_t m_size;

        /** Used to return at most m_size bytes or an exception. */
        ServiceResult<SharedBuffer> m_result;
      };

      /** The operations performed by this reader. */
      using Operation = std::variant<PollOperation, ReadOperation>;

      /** The queue used to receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestReader.
       * @param operations Receives operations performed on this reader.
       */
      explicit TestReader(
        ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept;

      ~TestReader();

      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination);
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size);

      /** Closes the reader and fails pending operations. */
      void close();

    private:
      mutable TestServiceClientOperationQueue<Operation> m_operations;

      TestReader(const TestReader&) = delete;
      TestReader& operator =(const TestReader&) = delete;
  };

  inline TestReader::TestReader(
    ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestReader::~TestReader() {
    close();
  }

  inline bool TestReader::poll() const {
    return m_operations.append_result<PollOperation, bool>();
  }

  template<IsBuffer B>
  std::size_t TestReader::read(Out<B> destination) {
    return read(out(destination), std::numeric_limits<std::size_t>::max());
  }

  template<IsBuffer B>
  std::size_t TestReader::read(Out<B> destination, std::size_t size) {
    auto data = m_operations.append_result<ReadOperation, SharedBuffer>(size);
    append(*destination, data);
    return data.get_size();
  }

  inline void TestReader::close() {
    m_operations.close();
  }
}

#endif
