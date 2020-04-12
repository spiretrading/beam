#include <atomic>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <boost/date_time.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/LocalDataStore.hpp"
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/IndexedSubscriptions.hpp"
#include "Beam/Queries/QueryClientPublisher.hpp"
#include "Beam/Queries/QueryResult.hpp"
#include "Beam/Queries/ShuttleQueryTypes.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Threading/TimerThreadPool.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/SynchronizedSet.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ApplicationClientBuilder = ServiceProtocolClientBuilder<
    MessageProtocol<std::unique_ptr<LocalClientChannel<SharedBuffer>>,
    BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
  using ApplicationClientHandler =
    ServiceProtocolClientHandler<ApplicationClientBuilder>;
  using ServerClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<LocalServerChannel<SharedBuffer>>,
    BinarySender<SharedBuffer>, NullEncoder>, std::unique_ptr<TriggerTimer>>;

  struct Data {
    int m_value;
    ptime m_timestamp;

    template<typename Shuttler>
    void Shuttle(Shuttler& shuttle, unsigned int version) {
      shuttle.Shuttle("value", m_value);
      shuttle.Shuttle("timestamp", m_timestamp);
    }
  };

  using DataQuery = BasicQuery<int>;
  using SequencedData = SequencedValue<Data>;
  using SequencedIndexedData = SequencedValue<IndexedValue<Data, int>>;
  using DataQueryResult = QueryResult<SequencedData>;

  BEAM_DEFINE_SERVICES(QueryServices,
    (QueryDataService, "QueryDataService", DataQueryResult, DataQuery, query));

  BEAM_DEFINE_MESSAGES(QueryMessages,
    (DataQueryMessage, "DataQueryMessage", SequencedIndexedData, data),
    (EndDataQueryMessage, "EndDataQueryMessage", int, index, int, id));

  template<typename ContainerType>
  class DataServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      DataServlet();

      void RegisterServices(
        Out<Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      struct DataEntry {
        std::unique_ptr<LiveTimer> m_timer;
        int m_index;
        Queries::Sequence m_sequence;

        DataEntry(std::unique_ptr<LiveTimer> timer, int index)
          : m_timer(std::move(timer)),
            m_index(index),
            m_sequence(Queries::Sequence::First()) {}
      };
      template<typename T>
      using Subscriptions = IndexedSubscriptions<T, int, ServiceProtocolClient>;
      Subscriptions<Data> m_dataSubscriptions;
      LocalDataStore<DataQuery, Data, EvaluatorTranslator<QueryTypes>>
        m_dataStore;
      std::atomic_bool m_timerState;
      TimerThreadPool m_timerThreadPool;
      std::vector<std::unique_ptr<DataEntry>> m_dataEntries;
      OpenState m_openState;
      RoutineTaskQueue m_taskQueue;

      void Shutdown();
      void OnDataRequest(
        RequestToken<ServiceProtocolClient, QueryDataService>& request,
        const DataQuery& query);
      void OnEndDataQuery(ServiceProtocolClient& client, int index, int id);
      void OnExpiry(Timer::Result result, DataEntry& entry);
  };

  struct MetaDataServlet {
    using Session = NullType;
    template<typename ContainerType>
    struct apply {
      using type = DataServlet<ContainerType>;
    };
  };

  template<typename ContainerType>
  DataServlet<ContainerType>::DataServlet()
    : m_timerState(true) {}

  template<typename ContainerType>
  void DataServlet<ContainerType>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterQueryTypes(Store(slots->GetRegistry()));
    RegisterQueryServices(Store(slots));
    RegisterQueryMessages(Store(slots));
    QueryDataService::AddRequestSlot(Store(slots),
      std::bind(&DataServlet::OnDataRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    AddMessageSlot<EndDataQueryMessage>(Store(slots),
      std::bind(&DataServlet::OnEndDataQuery, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_dataSubscriptions.RemoveAll(client);
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    auto rd = std::random_device();
    auto randomizer = std::default_random_engine(rd());
    auto distribution = std::uniform_int_distribution<std::uint64_t>();
    for(auto i = 0; i < 200; ++i) {
      auto interval = milliseconds(10 * (rand() % 100));
      auto entry = std::make_unique<DataEntry>(
        std::make_unique<LiveTimer>(interval, Ref(m_timerThreadPool)), i);
      entry->m_timer->GetPublisher().Monitor(m_taskQueue.GetSlot<Timer::Result>(
        std::bind(&DataServlet::OnExpiry, this, std::placeholders::_1,
        std::ref(*entry))));
      m_dataEntries.push_back(std::move(entry));
    }
    m_openState.SetOpen();
    for(auto& entry : m_dataEntries) {
      entry->m_timer->Start();
    }
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::Shutdown() {
    m_timerState = false;
    for(auto& entry : m_dataEntries) {
      entry->m_timer->Cancel();
    }
    m_openState.SetClosed();
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::OnDataRequest(
      RequestToken<ServiceProtocolClient, QueryDataService>& request,
      const DataQuery& query) {
    auto filter = Translate<EvaluatorTranslator<QueryTypes>>(query.GetFilter());
    auto result = DataQueryResult();
    result.m_queryId = m_dataSubscriptions.Initialize(query.GetIndex(),
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore.Load(query);
    m_dataSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const auto& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::OnEndDataQuery(ServiceProtocolClient& client,
      int index, int id) {
    m_dataSubscriptions.End(index, id);
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::OnExpiry(Timer::Result result,
      DataEntry& entry) {
    auto data = Data();
    data.m_value = entry.m_index;
    data.m_timestamp = microsec_clock::universal_time();
    entry.m_sequence = Increment(entry.m_sequence);
    auto value = SequencedValue(IndexedValue(data, entry.m_index),
      entry.m_sequence);
    m_dataStore.Store(value);
    m_dataSubscriptions.Publish(value,
      [&] (const auto& clients) {
        Beam::Services::BroadcastRecordMessage<DataQueryMessage>(
          clients, value);
      });
    if(m_timerState) {
      entry.m_timer->Start();
    }
  }

  using DataServletContainer = ServiceProtocolServletContainer<MetaDataServlet,
    LocalServerConnection<SharedBuffer>*, BinarySender<SharedBuffer>,
    NullEncoder, std::unique_ptr<TriggerTimer>>;
}

int main() {
  auto server = LocalServerConnection<SharedBuffer>();
  auto servlet = DataServletContainer(Initialize(), &server,
    [] {
      return std::make_unique<TriggerTimer>();
    });
  auto routines = RoutineHandlerGroup();
  auto timerThreadPool = TimerThreadPool();
  auto count = std::atomic_int(0);
  routines.Spawn(
    [&] {
      while(!ReceivedKillEvent()) {
        auto timer = LiveTimer(seconds(10), Ref(timerThreadPool));
        auto clients = rand() % 200;
        for(auto i = 0; i < clients; ++i) {
          routines.Spawn(
            [&] {
              ++count;
              std::cout << "Start: " << count << std::endl;
              auto clientHandler = ApplicationClientHandler(
                Initialize(
                [&] {
                  return std::make_unique<LocalClientChannel<SharedBuffer>>(
                    "dummy", Ref(server));
                },
                [] {
                  return std::make_unique<TriggerTimer>();
                }));
              RegisterQueryTypes(Store(clientHandler.GetSlots().GetRegistry()));
              RegisterQueryServices(Store(clientHandler.GetSlots()));
              RegisterQueryMessages(Store(clientHandler.GetSlots()));
              auto publisher = QueryClientPublisher<Data, DataQuery,
                EvaluatorTranslator<QueryTypes>,
                ServiceProtocolClientHandler<ApplicationClientBuilder>,
                QueryDataService, EndDataQueryMessage>(Ref(clientHandler));
              publisher.AddMessageHandler<DataQueryMessage>();
              clientHandler.Open();
              auto timer = LiveTimer(milliseconds(100), Ref(timerThreadPool));
              auto duration = 10 * (rand() % 20);
              for(auto i = 0; i < duration; ++i) {
                auto query = DataQuery();
                query.SetIndex(rand() % 200);
                query.SetRange(Range::Total());
                query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 1000);
                auto queue = std::make_shared<Queue<Data>>();
                publisher.SubmitQuery(query, queue);
                timer.Start();
                timer.Wait();
              }
              --count;
              std::cout << "Stop: " << count << std::endl;
              clientHandler.Close();
            });
        }
        timer.Start();
        timer.Wait();
      }
    });
  servlet.Open();
  routines.Wait();
  servlet.Close();
  routines.Wait();
  std::cout << "Done" << std::endl;
}
