#include "Beam/QueriesTests/TestDataStoreTester.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/QueriesTests/TestDataStore.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::gregorian;
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

  void Open(DataStore& dataStore) {
    auto operations =
      std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
    dataStore.GetOperationPublisher().Monitor(operations);
    auto result = Async<void>();
    auto openRoutine = RoutineHandler(Spawn(
      [&] {
        try {
          dataStore.Open();
          result.GetEval().SetResult();
        } catch(const std::exception&) {
          result.GetEval().SetException(std::current_exception());
        }
      }));
    auto operation = operations->Top();
    operations->Pop();
    auto openOperation = std::get_if<DataStore::OpenOperation>(&*operation);
    CPPUNIT_ASSERT(openOperation);
    openOperation->m_result.SetResult();
    CPPUNIT_ASSERT_NO_THROW(result.Get());
  }
}

void TestDataStoreTester::TestOpenException() {
  auto dataStore = DataStore();
  auto operations =
    std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
  dataStore.GetOperationPublisher().Monitor(operations);
  auto result = Async<void>();
  auto openRoutine = RoutineHandler(Spawn(
    [&] {
      try {
        dataStore.Open();
        result.GetEval().SetResult();
      } catch(const std::exception&) {
        result.GetEval().SetException(std::current_exception());
      }
    }));
  auto operation = operations->Top();
  operations->Pop();
  auto openOperation = std::get_if<DataStore::OpenOperation>(&*operation);
  CPPUNIT_ASSERT(openOperation);
  openOperation->m_result.SetException(ConnectException());
  CPPUNIT_ASSERT_THROW(result.Get(), ConnectException);
}

void TestDataStoreTester::TestStore() {
  auto dataStore = DataStore();
  Open(dataStore);
  auto operations =
    std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
  dataStore.GetOperationPublisher().Monitor(operations);
  auto result = Async<void>();
  auto storeRoutine = RoutineHandler(Spawn(
    [&] {
      try {
        dataStore.Store(SequencedValue(IndexedValue(
          Entry(123, ptime(date(2018, 5, 3))), std::string("hello")),
          Sequence(110)));
        result.GetEval().SetResult();
      } catch(const std::exception&) {
        result.GetEval().SetException(std::current_exception());
      }
    }));
  auto operation = operations->Top();
  operations->Pop();
  auto storeOperation = std::get_if<DataStore::StoreOperation>(&*operation);
  CPPUNIT_ASSERT(storeOperation);
  storeOperation->m_result.SetResult();
  CPPUNIT_ASSERT_NO_THROW(result.Get());
}

void TestDataStoreTester::TestStoreException() {
  auto dataStore = DataStore();
  Open(dataStore);
  auto operations =
    std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
  dataStore.GetOperationPublisher().Monitor(operations);
  auto result = Async<void>();
  auto storeRoutine = RoutineHandler(Spawn(
    [&] {
      try {
        dataStore.Store(SequencedValue(IndexedValue(
          Entry(123, ptime(date(2018, 5, 3))), std::string("hello")),
          Sequence(110)));
        result.GetEval().SetResult();
      } catch(const std::exception&) {
        result.GetEval().SetException(std::current_exception());
      }
    }));
  auto operation = operations->Top();
  operations->Pop();
  auto storeOperation = std::get_if<DataStore::StoreOperation>(&*operation);
  CPPUNIT_ASSERT(storeOperation);
  storeOperation->m_result.SetException(std::runtime_error("Store failed."));
  CPPUNIT_ASSERT_THROW(result.Get(), std::runtime_error);
}

void TestDataStoreTester::TestLoad() {
  auto dataStore = DataStore();
  Open(dataStore);
  auto operations =
    std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
  dataStore.GetOperationPublisher().Monitor(operations);
  auto result = Async<std::vector<SequencedEntry>>();
  auto query = BasicQuery<std::string>();
  query.SetIndex("index");
  query.SetSnapshotLimit(SnapshotLimit::Unlimited());
  auto loadRoutine = RoutineHandler(Spawn(
    [&] {
      try {
        result.GetEval().SetResult(dataStore.Load(query));
      } catch(const std::exception&) {
        result.GetEval().SetException(std::current_exception());
      }
    }));
  auto operation = operations->Top();
  operations->Pop();
  auto loadOperation = std::get_if<DataStore::LoadOperation>(&*operation);
  CPPUNIT_ASSERT(loadOperation);
  auto series = std::vector<SequencedEntry>();
  for(auto i = 0; i < 10; ++i) {
    series.push_back(SequencedValue(IndexedValue(
      Entry(i, ptime(date(2018, i % 13 + 1, 3))), std::string("hello")),
      Sequence(i + 100)));
  }
  loadOperation->m_result.SetResult(series);
  CPPUNIT_ASSERT(result.Get() == series);
}

void TestDataStoreTester::TestLoadException() {
  auto dataStore = DataStore();
  Open(dataStore);
  auto operations =
    std::make_shared<Queue<std::shared_ptr<DataStore::Operation>>>();
  dataStore.GetOperationPublisher().Monitor(operations);
  auto result = Async<std::vector<SequencedEntry>>();
  auto query = BasicQuery<std::string>();
  query.SetIndex("index");
  query.SetSnapshotLimit(SnapshotLimit::Unlimited());
  auto loadRoutine = RoutineHandler(Spawn(
    [&] {
      try {
        result.GetEval().SetResult(dataStore.Load(query));
      } catch(const std::exception&) {
        result.GetEval().SetException(std::current_exception());
      }
    }));
  auto operation = operations->Top();
  operations->Pop();
  auto loadOperation = std::get_if<DataStore::LoadOperation>(&*operation);
  CPPUNIT_ASSERT(loadOperation);
  loadOperation->m_result.SetException(std::runtime_error("Load failed."));
  CPPUNIT_ASSERT_THROW(result.Get(), std::runtime_error);
}
