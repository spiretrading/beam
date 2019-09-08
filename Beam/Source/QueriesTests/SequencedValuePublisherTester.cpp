#include "Beam/QueriesTests/SequencedValuePublisherTester.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

SequencedValuePublisherTester::PublisherEntry::PublisherEntry(
    const TestQuery& query)
    : m_queue(std::make_shared<Queue<SequencedValue<string>>>()),
      m_publisher(query, Translate(query.GetFilter()), m_queue) {}

void SequencedValuePublisherTester::TestPublishWithTotalRange() {
  TestQuery query;
  query.SetRange(Range::Total());
  unique_ptr<PublisherEntry> publisher = MakePublisher(query);
  InitializeSnapshot(*publisher, vector<SequencedValue<string>>());
  SequencedValue<string> helloValue("hello", Sequence(3));
  publisher->m_publisher.Push(helloValue);
  ExpectValue(*publisher->m_queue, helloValue);
  SequencedValue<string> worldValue("world", Sequence(3));
  publisher->m_publisher.Push(worldValue);
  CPPUNIT_ASSERT(publisher->m_queue->IsEmpty());
  SequencedValue<string> goodbyeValue("goodbye", Sequence(2));
  publisher->m_publisher.Push(goodbyeValue);
  CPPUNIT_ASSERT(publisher->m_queue->IsEmpty());
  SequencedValue<string> skyValue("goodbye", Sequence(4));
  publisher->m_publisher.Push(skyValue);
  ExpectValue(*publisher->m_queue, skyValue);
}

void SequencedValuePublisherTester::TestSnapshotWithTotalRange() {
  TestQuery query;
  query.SetRange(Range::Total());
  unique_ptr<PublisherEntry> publisher = MakePublisher(query);
  vector<SequencedValue<string>> snapshot;
  snapshot.push_back(SequencedValue("hello", Sequence(3)));
  InitializeSnapshot(*publisher, snapshot);
  SequencedValue<string> worldValue("world", Sequence(3));
  publisher->m_publisher.Push(worldValue);
  CPPUNIT_ASSERT(publisher->m_queue->IsEmpty());
  SequencedValue<string> goodbyeValue("goodbye", Sequence(2));
  publisher->m_publisher.Push(goodbyeValue);
  CPPUNIT_ASSERT(publisher->m_queue->IsEmpty());
  SequencedValue<string> skyValue("goodbye", Sequence(4));
  publisher->m_publisher.Push(skyValue);
  ExpectValue(*publisher->m_queue, skyValue);
}

unique_ptr<SequencedValuePublisherTester::PublisherEntry>
    SequencedValuePublisherTester::MakePublisher(const TestQuery& query) {
  unique_ptr<Evaluator> filter = Translate(query.GetFilter());
  unique_ptr<PublisherEntry> publisherEntry =
    std::make_unique<PublisherEntry>(query);
  return publisherEntry;
}

void SequencedValuePublisherTester::InitializeSnapshot(
    PublisherEntry& publisher,
    const vector<SequencedValue<string>>& snapshot) {
  vector<SequencedValue<string>> pushSnapshot = snapshot;
  publisher.m_publisher.BeginSnapshot();
  publisher.m_publisher.PushSnapshot(pushSnapshot.begin(), pushSnapshot.end());
  publisher.m_publisher.EndSnapshot(123);
  for(const SequencedValue<string>& value : snapshot) {
    ExpectValue(*publisher.m_queue, value);
  }
}

void SequencedValuePublisherTester::ExpectValue(
    Queue<SequencedValue<string>>& queue,
    const SequencedValue<string>& expectedValue) {
  SequencedValue<string> value = queue.Top();
  queue.Pop();
  CPPUNIT_ASSERT_EQUAL(value, expectedValue);
}
