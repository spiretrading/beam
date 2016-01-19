#ifndef BEAM_SEQUENCEDVALUEPUBLISHERTESTER_HPP
#define BEAM_SEQUENCEDVALUEPUBLISHERTESTER_HPP
#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SequencedValuePublisher.hpp"
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class SequencedValuePublisherTester
      \brief Tests the SequencedValuePublisher class.
   */
  class SequencedValuePublisherTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of query used for testing.
      typedef BasicQuery<int> TestQuery;

      //! The type of SequencedValuePublisher used for testing.
      typedef SequencedValuePublisher<TestQuery, std::string>
        TestSequencedValuePublisher;

      //! Make a query with total Range.
      //! Make a publisher.
      //! Initialize an empty snapshot.
      //! Push a value V1 with Sequence S1 to the publisher.
      //! Expect to receive that V1 in the queue.
      //! Push a value V2 with a Sequence S1 to the publisher.
      //! Expect the queue to be empty.
      //! Push a value V3 with a Sequence S2 < S1 to the publisher.
      //! Expect the queue to be empty.
      //! Push a value V4 with Sequence S3 > S1 to the publisher.
      //! Expect to receive V4 in the queue.
      void TestPublishWithTotalRange();

      //! Make a query with total Range.
      //! Make a publisher.
      //! Initialize the publisher with value V1 and Sequence S1.
      //! Expect to receive that V1 in the queue.
      //! Push a value V2 with a Sequence S1 to the publisher.
      //! Expect the queue to be empty.
      //! Push a value V3 with a Sequence S2 < S1 to the publisher.
      //! Expect the queue to be empty.
      //! Push a value V4 with Sequence S3 > S1 to the publisher.
      //! Expect to receive V4 in the queue.
      void TestSnapshotWithTotalRange();

    private:
      CPPUNIT_TEST_SUITE(SequencedValuePublisherTester);
        CPPUNIT_TEST(TestPublishWithTotalRange);
        CPPUNIT_TEST(TestSnapshotWithTotalRange);
      BEAM_CPPUNIT_TEST_SUITE_END();
      struct PublisherEntry {
        std::shared_ptr<Queue<SequencedValue<std::string>>> m_queue;
        TestSequencedValuePublisher m_publisher;

        PublisherEntry(const TestQuery& query);
      };

      std::unique_ptr<PublisherEntry> MakePublisher(const TestQuery& query);
      void InitializeSnapshot(PublisherEntry& publisher,
        const std::vector<SequencedValue<std::string>>& snapshot);
      void ExpectValue(Queue<SequencedValue<std::string>>& queue,
        const SequencedValue<std::string>& expectedValue);
  };
}
}
}

#endif
