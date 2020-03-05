#include "Beam/QueriesTests/TestDataStoreTester.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/QueriesTests/TestDataStore.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;

    Entry() = default;
    Entry(int value, ptime timestamp);
    bool operator ==(const Entry& rhs) const;
  };

  using DataStore = TestDataStore<BasicQuery<std::string>, Entry>;
  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry = SequencedValue<IndexedValue<Entry,
    std::string>>;

  Entry::Entry(int value, ptime timestamp)
    : m_value(value),
      m_timestamp(timestamp) {}

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }
}

void TestDataStoreTester::TestStore() {
  auto dataStore = DataStore();
  auto operations =
    std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
  dataStore.GetOperationPublisher().Monitor(operations);
  auto openRoutine = RoutineHandler(Spawn(
    [&] {
      dataStore.Open();
    }));
  auto operation = operations->Top();
  operations->Pop();
  auto openOperation = std::get_if<DataStore::OpenOperation>(&*operation);
  CPPUNIT_ASSERT(openOperation);
  openOperation->m_result.SetResult();
}
