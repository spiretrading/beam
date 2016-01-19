#include "Beam/ReactorsTests/PublisherReactorTester.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueuePublisher.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/TriggeredReactor.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::Reactors::Tests;
using namespace Beam::Routines;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;

void PublisherReactorTester::TestOneByOneUpdates() {
  Async<void> updateToken;
  MultiQueueWriter<int> publisher;
  auto reactor = MakePublisherReactor(&publisher);
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  reactor->ConnectEventSignal(
    [&] {
      updateToken.GetEval().SetResult();
    });
  publisher.Push(123);
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT_EQUAL(123, reactor->Eval());
  publisher.Push(321);
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  auto sequenceA = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT_EQUAL(321, reactor->Eval());
  publisher.Push(10);
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  auto sequenceB = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceB > sequenceA);
  CPPUNIT_ASSERT_EQUAL(10, reactor->Eval());
  publisher.Push(20);
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  auto sequenceC = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceC > sequenceB);
  CPPUNIT_ASSERT_EQUAL(20, reactor->Eval());
  publisher.Push(30);
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  auto sequenceD = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceD > sequenceC);
  CPPUNIT_ASSERT_EQUAL(30, reactor->Eval());
  publisher.Break();
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  auto sequenceE = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceE == sequenceD);
  CPPUNIT_ASSERT(reactor->IsComplete());
  CPPUNIT_ASSERT_EQUAL(30, reactor->Eval());
}

void PublisherReactorTester::TestMultipleUpdates() {
  Async<void> updateToken;
  MultiQueueWriter<int> publisher;
  auto reactor = MakePublisherReactor(&publisher);
  reactor->Commit();
  CPPUNIT_ASSERT(reactor->IsInitializing());
  int updateCount = 4;
  reactor->ConnectEventSignal(
    [&] {
      --updateCount;
      if(updateCount == 0) {
        updateToken.GetEval().SetResult();
      }
    });
  publisher.Push(123);
  publisher.Push(456);
  publisher.Push(789);
  publisher.Break();
  updateToken.Get();
  updateToken.Reset();
  reactor->Execute();
  reactor->Commit();
  auto sequenceA = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(reactor->IsInitialized());
  CPPUNIT_ASSERT_EQUAL(123, reactor->Eval());
  reactor->Execute();
  reactor->Commit();
  auto sequenceB = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceB > sequenceA);
  CPPUNIT_ASSERT_EQUAL(456, reactor->Eval());
  reactor->Execute();
  reactor->Commit();
  auto sequenceC = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceC > sequenceB);
  CPPUNIT_ASSERT_EQUAL(789, reactor->Eval());
  reactor->Execute();
  reactor->Commit();
  auto sequenceD = reactor->GetSequenceNumber();
  CPPUNIT_ASSERT(sequenceD == sequenceC);
  CPPUNIT_ASSERT_EQUAL(789, reactor->Eval());
  CPPUNIT_ASSERT(reactor->IsComplete());
}
