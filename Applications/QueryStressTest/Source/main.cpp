#include <atomic>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <boost/date_time.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Collections/SynchronizedSet.hpp"
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
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/TimeService/LiveTimer.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  using ApplicationClientBuilder = ServiceProtocolClientBuilder<
    MessageProtocol<std::unique_ptr<LocalClientChannel>,
      BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
  using ApplicationClientHandler =
    ServiceProtocolClientHandler<ApplicationClientBuilder>;
  using ServerClient = ServiceProtocolClient<
    MessageProtocol<std::unique_ptr<LocalServerChannel>,
      BinarySender<SharedBuffer>, NullEncoder>, std::unique_ptr<TriggerTimer>>;

  struct Data {
    int m_value;
    ptime m_timestamp;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("value", m_value);
      shuttle.shuttle("timestamp", m_timestamp);
    }
  };

  using DataQuery = BasicQuery<int>;
  using SequencedData = SequencedValue<Data>;
  using SequencedIndexedData = SequencedValue<IndexedValue<Data, int>>;
  using DataQueryResult = QueryResult<SequencedData>;

  BEAM_DEFINE_SERVICES(query_services,
    (QueryDataService, "QueryDataService", DataQueryResult,
      (DataQuery, query)));

  BEAM_DEFINE_MESSAGES(query_messages,
    (DataQueryMessage, "DataQueryMessage", (SequencedIndexedData, data)),
    (EndDataQueryMessage, "EndDataQueryMessage", (int, index), (int, id)));

  template<typename ContainerType>
  class DataServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      DataServlet();

      void register_services(Out<ServiceSlots<ServiceProtocolClient>> slots);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      struct DataEntry {
        std::unique_ptr<LiveTimer> m_timer;
        int m_index;
        Beam::Sequence m_sequence;

        DataEntry(std::unique_ptr<LiveTimer> timer, int index)
          : m_timer(std::move(timer)),
            m_index(index),
            m_sequence(Beam::Sequence::FIRST) {}
      };
      template<typename T>
      using Subscriptions = IndexedSubscriptions<T, int, ServiceProtocolClient>;
      Subscriptions<Data> m_subscriptions;
      LocalDataStore<DataQuery, Data, EvaluatorTranslator<QueryTypes>>
        m_data_store;
      std::atomic_bool m_timer_state;
      std::vector<std::unique_ptr<DataEntry>> m_data_entries;
      OpenState m_open_state;
      RoutineTaskQueue m_tasks;

      void on_data_request(
        RequestToken<ServiceProtocolClient, QueryDataService>& request,
        const DataQuery& query);
      void on_end_data_query(ServiceProtocolClient& client, int index, int id);
      void on_expiry(Timer::Result result, DataEntry& entry);
  };

  struct MetaDataServlet {
    using Session = NullSession;
    template<typename ContainerType>
    struct apply {
      using type = DataServlet<ContainerType>;
    };
  };

  template<typename ContainerType>
  DataServlet<ContainerType>::DataServlet()
      : m_timer_state(true) {
    auto rd = std::random_device();
    auto randomizer = std::default_random_engine(rd());
    auto distribution = std::uniform_int_distribution<std::uint64_t>();
    for(auto i = 0; i < 200; ++i) {
      auto interval = milliseconds(10 * (std::rand() % 100));
      auto entry =
        std::make_unique<DataEntry>(std::make_unique<LiveTimer>(interval), i);
      entry->m_timer->get_publisher().monitor(m_tasks.get_slot<Timer::Result>(
        std::bind(&DataServlet::on_expiry, this, std::placeholders::_1,
          std::ref(*entry))));
      m_data_entries.push_back(std::move(entry));
    }
    for(auto& entry : m_data_entries) {
      entry->m_timer->start();
    }
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::register_services(
      Out<ServiceSlots<ServiceProtocolClient>> slots) {
    register_query_types(out(slots->get_registry()));
    register_query_services(out(slots));
    register_query_messages(out(slots));
    QueryDataService::add_request_slot(
      out(slots), std::bind_front(&DataServlet::on_data_request, this));
    add_message_slot<EndDataQueryMessage>(
      out(slots), std::bind_front(&DataServlet::on_end_data_query, this));
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::handle_close(ServiceProtocolClient& client) {
    m_subscriptions.remove_all(client);
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_timer_state = false;
    for(auto& entry : m_data_entries) {
      entry->m_timer->cancel();
    }
    m_open_state.close();
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::on_data_request(
      RequestToken<ServiceProtocolClient, QueryDataService>& request,
      const DataQuery& query) {
    auto filter =
      translate<EvaluatorTranslator<QueryTypes>>(query.get_filter());
    auto result = DataQueryResult();
    result.m_id = m_subscriptions.init(query.get_index(), request.get_client(),
      query.get_range(), std::move(filter));
    result.m_snapshot = m_data_store.load(query);
    m_subscriptions.commit(query.get_index(), std::move(result),
      [&] (const auto& result) {
        request.set(result);
      });
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::on_end_data_query(
      ServiceProtocolClient& client, int index, int id) {
    m_subscriptions.end(index, id);
  }

  template<typename ContainerType>
  void DataServlet<ContainerType>::on_expiry(
      Timer::Result result, DataEntry& entry) {
    auto data = Data();
    data.m_value = entry.m_index;
    data.m_timestamp = microsec_clock::universal_time();
    entry.m_sequence = increment(entry.m_sequence);
    auto value =
      SequencedValue(IndexedValue(data, entry.m_index), entry.m_sequence);
    m_data_store.store(value);
    m_subscriptions.publish(value, [&] (const auto& clients) {
      broadcast_record_message<DataQueryMessage>(clients, value);
    });
    if(m_timer_state) {
      entry.m_timer->start();
    }
  }

  using DataServletContainer = ServiceProtocolServletContainer<
    MetaDataServlet, LocalServerConnection*, BinarySender<SharedBuffer>,
    NullEncoder, std::unique_ptr<TriggerTimer>>;
}

int main() {
  auto server = LocalServerConnection();
  auto servlet = DataServletContainer(init(), &server, [] {
    return std::make_unique<TriggerTimer>();
  });
  auto routines = RoutineHandlerGroup();
  auto count = std::atomic_int(0);
  routines.spawn([&] {
    while(!received_kill_event()) {
      auto timer = LiveTimer(seconds(10));
      auto clients = std::rand() % 200;
      for(auto i = 0; i < clients; ++i) {
        routines.spawn([&] {
          ++count;
          std::cout << "Start: " << count << std::endl;
          auto client_handler = ApplicationClientHandler(init(
            [&] {
              return std::make_unique<LocalClientChannel>("dummy", server);
            },
            [] {
              return std::make_unique<TriggerTimer>();
            }));
          register_query_types(out(client_handler.get_slots().get_registry()));
          register_query_services(out(client_handler.get_slots()));
          register_query_messages(out(client_handler.get_slots()));
          auto publisher = QueryClientPublisher<
            Data, DataQuery, EvaluatorTranslator<QueryTypes>,
            ServiceProtocolClientHandler<ApplicationClientBuilder>,
            QueryDataService, EndDataQueryMessage>(Ref(client_handler));
          publisher.add_message_handler<DataQueryMessage>();
          auto timer = LiveTimer(milliseconds(100));
          auto duration = 10 * (std::rand() % 20);
          for(auto i = 0; i < duration; ++i) {
            auto query = DataQuery();
            query.set_index(std::rand() % 200);
            query.set_range(Range::TOTAL);
            query.set_snapshot_limit(SnapshotLimit::from_tail(1000));
            auto queue = std::make_shared<Queue<Data>>();
            publisher.submit(query, queue);
            timer.start();
            timer.wait();
          }
          --count;
          std::cout << "Stop: " << count << std::endl;
          client_handler.close();
        });
      }
      timer.start();
      timer.wait();
    }
  });
  routines.wait();
  servlet.close();
  routines.wait();
  std::cout << "Done" << std::endl;
}
