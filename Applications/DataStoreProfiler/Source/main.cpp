module;
#include "Prelude.hpp"
#include <cmath>
#include <thread>
#include <Viper/MySql/Connection.hpp>
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Sql/MySqlConfig.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

module Beam;

namespace Beam {

  /** Stores a dummy value used to profile data stores. */
  struct Entry {

    /** The index/name of the entry. */
    std::string m_name;

    /** Item A. */
    int m_item_a;

    /** Item B. */
    std::int64_t m_item_b;

    /** Item C. */
    std::int64_t m_item_c;

    /** Item D. */
    std::string m_item_d;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;
  };

  /** Returns the Entry's timestamp. */
  inline auto& get_timestamp(Entry& entry) {
    return entry.m_timestamp;
  }

  /** Returns the Entry's timestamp. */
  inline auto& get_timestamp(const Entry& entry) {
    return entry.m_timestamp;
  }
}


namespace Beam {
  using SequencedEntry = SequencedValue<Entry>;
  using IndexedEntry = IndexedValue<Entry, std::string>;
  using SequencedIndexedEntry = SequencedValue<IndexedEntry>;
  using EntryQuery = BasicQuery<std::string>;

  /**
   * Builds an EntryQuery for real time data with a snapshot containing the
   * most recent value.
   * @param name The name of the entry to query.
   */
  inline auto query_real_time_with_snapshot(std::string name) {
    auto query = EntryQuery();
    query.set_index(std::move(name));
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::Type::TAIL, 1);
    query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    return query;
  }
}


namespace Beam {

  /**
   * Wraps a data store to adapt it to the query interface.
   * @tparam D The data store to wrap.
   */
  template<typename D>
  class DataStoreQueryWrapper {
    public:
      using Query = EntryQuery;
      using Value = Entry;
      using Index = std::string;
      using DataStore = dereference_t<D>;
      using SequencedValue = Beam::SequencedValue<Value>;
      using IndexedValue =
        Beam::SequencedValue<Beam::IndexedValue<Value, Index>>;

      template<Initializes<D> DF>
      explicit DataStoreQueryWrapper(DF&& data_store);

      ~DataStoreQueryWrapper();

      std::vector<SequencedEntry> load(const EntryQuery& query);
      void store(const IndexedValue& value);
      void store(const std::vector<IndexedValue>& values);
      void close();

    private:
      local_ptr_t<D> m_data_store;

      DataStoreQueryWrapper(const DataStoreQueryWrapper&) = delete;
      DataStoreQueryWrapper& operator =(const DataStoreQueryWrapper&) = delete;
  };

  template<typename D>
  DataStoreQueryWrapper(D&&) -> DataStoreQueryWrapper<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  DataStoreQueryWrapper<D>::DataStoreQueryWrapper(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)) {}

  template<typename D>
  DataStoreQueryWrapper<D>::~DataStoreQueryWrapper() {
    close();
  }

  template<typename D>
  std::vector<SequencedEntry> DataStoreQueryWrapper<D>::load(
      const EntryQuery& query) {
    return m_data_store->load_entries(query);
  }

  template<typename D>
  void DataStoreQueryWrapper<D>::store(const IndexedValue& value) {
    m_data_store->store(value);
  }

  template<typename D>
  void DataStoreQueryWrapper<D>::store(const std::vector<IndexedValue>& values) {
    m_data_store->store(values);
  }

  template<typename D>
  void DataStoreQueryWrapper<D>::close() {
    m_data_store->close();
  }
}


namespace Beam {

  /**
   * Performs asyncronous writes to an underlying data store.
   * @tparam D The underlying data store to commit the data to.
   */
  template<typename D>
  class AsyncProfileDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = dereference_t<D>;

      /**
       * Constructs an AsyncProfileDataStore.
       * @param data_store Initializes the data store to commit data to.
       */
      template<Initializes<D> DF>
      explicit AsyncProfileDataStore(DF&& data_store);

      ~AsyncProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      AsyncDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        EvaluatorTranslator<QueryTypes>> m_async_data_store;
      OpenState m_open_state;

      AsyncProfileDataStore(const AsyncProfileDataStore&) = delete;
      AsyncProfileDataStore& operator =(const AsyncProfileDataStore&) = delete;
  };

  template<typename D>
  AsyncProfileDataStore(D&&) -> AsyncProfileDataStore<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  AsyncProfileDataStore<D>::AsyncProfileDataStore(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)),
      m_async_data_store(&*m_data_store) {}

  template<typename D>
  AsyncProfileDataStore<D>::~AsyncProfileDataStore() {
    close();
  }

  template<typename D>
  void AsyncProfileDataStore<D>::clear() {
    m_data_store->clear();
  }

  template<typename D>
  std::vector<SequencedEntry> AsyncProfileDataStore<D>::load_entries(
      const EntryQuery& query) {
    return m_async_data_store.load(query);
  }

  template<typename D>
  void AsyncProfileDataStore<D>::store(const SequencedIndexedEntry& entry) {
    m_async_data_store.store(entry);
  }

  template<typename D>
  void AsyncProfileDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_async_data_store.close();
    m_data_store->close();
    m_open_state.close();
  }
}


namespace Beam {

  /**
   * Buffers writes to an underlying data store.
   * @tparam D The underlying data store to commit the data to.
   */
  template<typename D>
  class BufferedProfileDataStore {
    public:

      /** The type of DataStore to buffer. */
      using BaseDataStore = dereference_t<D>;

      /**
       * Constructs a BufferedProfileDataStore.
       * @param data_store Initializes the data store to commit data to.
       * @param buffer_size The number of messages to buffer before committing
       *        to the <i>data_store</i>.
       */
      template<Initializes<D> DF>
      BufferedProfileDataStore(DF&& data_store, std::size_t buffer_size);

      ~BufferedProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      BufferedDataStore<DataStoreQueryWrapper<BaseDataStore*>,
        EvaluatorTranslator<QueryTypes>> m_buffered_data_store;
      OpenState m_open_state;

      BufferedProfileDataStore(const BufferedProfileDataStore&) = delete;
      BufferedProfileDataStore& operator =(
        const BufferedProfileDataStore&) = delete;
  };

  template<typename D>
  BufferedProfileDataStore(D&&, std::size_t) ->
    BufferedProfileDataStore<std::remove_cvref_t<D>>;

  template<typename D>
  template<Initializes<D> DF>
  BufferedProfileDataStore<D>::BufferedProfileDataStore(
    DF&& data_store, std::size_t buffer_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_buffered_data_store(&*m_data_store, buffer_size) {}

  template<typename D>
  BufferedProfileDataStore<D>::~BufferedProfileDataStore() {
    close();
  }

  template<typename D>
  void BufferedProfileDataStore<D>::clear() {
    m_data_store->clear();
  }

  template<typename D>
  std::vector<SequencedEntry> BufferedProfileDataStore<D>::load_entries(
      const EntryQuery& query) {
    return m_buffered_data_store.load(query);
  }

  template<typename D>
  void BufferedProfileDataStore<D>::store(const SequencedIndexedEntry& entry) {
    m_buffered_data_store.store(entry);
  }

  template<typename D>
  void BufferedProfileDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_buffered_data_store.close();
    m_data_store->close();
    m_open_state.close();
  }
}


namespace Beam {

  /** Stores data in a MySQL database. */
  class MySqlProfileDataStore {
    public:

      /**
       * Constructs a MySqlProfileDataStore.
       * @param address The IP address of the MySQL database to connect to.
       * @param schema The name of the schema.
       * @param username The username to connect as.
       * @param password The password associated with the <i>username</i>.
       */
      MySqlProfileDataStore(IpAddress address, std::string schema,
        std::string username, std::string password);

      ~MySqlProfileDataStore();

      void clear();
      std::vector<SequencedEntry> load_entries(const EntryQuery& query);
      void store(const SequencedIndexedEntry& entry);
      void store(const std::vector<SequencedIndexedEntry>& entries);
      void close();

    private:
      template<typename V, typename I>
      using DataStore = SqlDataStore<
        SqlConnection<Viper::MySql::Connection>, V, I, SqlTranslator>;
      DatabaseConnectionPool<SqlConnection<Viper::MySql::Connection>>
        m_reader_pool;
      DatabaseConnectionPool<SqlConnection<Viper::MySql::Connection>>
        m_writer_pool;
      DataStore<Viper::Row<Entry>, Viper::Row<std::string>> m_data_store;
      OpenState m_open_state;

      MySqlProfileDataStore(const MySqlProfileDataStore&) = delete;
      MySqlProfileDataStore& operator =(const MySqlProfileDataStore&) = delete;
      static Viper::Row<Entry> build_value_row();
      static Viper::Row<std::string> build_index_row();
  };

  inline MySqlProfileDataStore::MySqlProfileDataStore(IpAddress address,
    std::string schema, std::string username, std::string password)
    : m_reader_pool(std::thread::hardware_concurrency(), [=] {
        auto connection = make_sql_connection(Viper::MySql::Connection(
          address.get_host(), address.get_port(), username, password, schema));
        connection->open();
        return connection;
      }),
      m_writer_pool(std::thread::hardware_concurrency(), [=] {
        auto connection = make_sql_connection(Viper::MySql::Connection(
          address.get_host(), address.get_port(), username, password, schema));
        connection->open();
        return connection;
      }),
      m_data_store("entries", build_value_row(), build_index_row(),
        Ref(m_reader_pool), Ref(m_writer_pool)) {}

  inline MySqlProfileDataStore::~MySqlProfileDataStore() {
    close();
  }

  inline void MySqlProfileDataStore::clear() {
    auto connection = m_writer_pool.load();
    connection->execute(Viper::truncate("entries"));
  }

  inline std::vector<SequencedEntry> MySqlProfileDataStore::load_entries(
      const EntryQuery& query) {
    return m_data_store.load(query);
  }

  inline void MySqlProfileDataStore::store(
      const SequencedIndexedEntry& entry) {
    return m_data_store.store(entry);
  }

  inline void MySqlProfileDataStore::store(
      const std::vector<SequencedIndexedEntry>& entries) {
    return m_data_store.store(entries);
  }

  inline void MySqlProfileDataStore::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_writer_pool.close();
    m_reader_pool.close();
    m_open_state.close();
  }

  inline Viper::Row<Entry> MySqlProfileDataStore::build_value_row() {
    return Viper::Row<Entry>().
      add_column("item_a", &Entry::m_item_a).
      add_column("item_b", &Entry::m_item_b).
      add_column("item_c", &Entry::m_item_c).
      add_column("item_d", Viper::varchar(100), &Entry::m_item_d);
  }

  inline Viper::Row<std::string> MySqlProfileDataStore::build_index_row() {
    return Viper::Row<std::string>().add_column(
      "name", Viper::VarCharDataType(16));
  }
}


using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct ProfileConfig {
    int m_index_count;
    int m_seed_count;
    int m_growth_factor;
    std::size_t m_buffer_size;
    int m_iterations;
    ptime m_start_time;
    time_duration m_time_step;
    std::vector<std::string> m_names;

    static auto parse(const YAML::Node& config);
  };

  auto ProfileConfig::parse(const YAML::Node& config) {
    auto profile_config = ProfileConfig();
    profile_config.m_index_count = extract<int>(config, "index_count");
    profile_config.m_seed_count = extract<int>(config, "seed_count");
    profile_config.m_growth_factor = extract<int>(config, "growth_factor");
    profile_config.m_buffer_size = extract<std::size_t>(config, "buffer_size");
    profile_config.m_iterations = extract<int>(config, "iterations");
    profile_config.m_start_time = extract<ptime>(config, "start_time");
    profile_config.m_time_step = extract<time_duration>(config, "time_step");
    for(auto i = 0; i < profile_config.m_index_count; ++i) {
      auto name = std::string();
      do {
        name.clear();
        for(auto j = 0; j < 4; ++j) {
          name += 'A' + static_cast<char>(std::rand() % 26);
        }
      } while(std::find(profile_config.m_names.begin(),
        profile_config.m_names.end(), name) != profile_config.m_names.end());
      profile_config.m_names.push_back(name);
    }
    return profile_config;
  }

  template<typename DataStore>
  void profile_writes(DataStore& data_store, const ProfileConfig& config) {
    data_store.clear();
    auto start = microsec_clock::universal_time();
    auto groups = static_cast<int>(std::ceil(std::log2(
      static_cast<double>(config.m_index_count) / config.m_seed_count)));
    auto range = static_cast<int>(std::pow(2, groups));
    auto timestamp = config.m_start_time;
    auto sequences = std::unordered_map<std::string, Beam::Sequence>();
    for(auto i = 0; i < config.m_iterations; ++i) {
      auto group = std::abs(
        static_cast<int>(std::log2(1 + (std::rand() % range))) - (groups - 1));
      auto lower_index =
        config.m_seed_count * static_cast<int>(std::pow(2, group) - 1);
      auto group_size =
        config.m_seed_count * static_cast<int>(std::pow(2, group));
      auto index = lower_index + (std::rand() % group_size);
      if(index >= config.m_index_count) {
        index = lower_index + (index - config.m_index_count);
      }
      auto entry = Entry(config.m_names[index], std::rand() % 100,
        std::rand() % 10000000, std::rand() % 10000000, "dummy", timestamp);
      auto& sequence = sequences[entry.m_name];
      sequence = increment(sequence);
      auto sequenced_indexed_entry =
        SequencedValue(IndexedValue(entry, entry.m_name), sequence);
      data_store.store(sequenced_indexed_entry);
      timestamp += config.m_time_step;
    }
    data_store.close();
    auto end = microsec_clock::universal_time();
    auto elapsed = end - start;
    auto rate = config.m_iterations / elapsed.total_seconds();
    std::cout << "profile_writes: " << (end - start) << " " << rate <<
      std::endl;
  }

  template<typename DataStore>
  void profile_reads(DataStore& data_store, const ProfileConfig& config) {
    auto end_timestamp =
      config.m_start_time + config.m_time_step * config.m_iterations;
    auto start = microsec_clock::universal_time();
    auto count = 0;
    for(auto i = 0; i < config.m_iterations; ++i) {
      auto& name = config.m_names[std::rand() % config.m_index_count];
      auto start_time = config.m_start_time + milliseconds(std::rand() %
        (end_timestamp - config.m_start_time).total_milliseconds());
      auto end_time = start_time + milliseconds(std::rand() %
        (end_timestamp - start_time).total_milliseconds());
      auto query = EntryQuery();
      query.set_index(name);
      query.set_range(start_time, end_time);
      query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
      auto result = data_store.load_entries(query);
      count += result.size();
    }
    data_store.close();
    auto end = microsec_clock::universal_time();
    auto elapsed = end - start;
    auto rate = config.m_iterations / elapsed.total_seconds();
    std::cout << "profile_reads: " << (end - start) << " " << rate <<
      std::endl;
  }

  void profile_buffered_data_store(
      const MySqlConfig& mysql_config, const ProfileConfig& profile_config) {
    std::cout << "BufferedDataStore" << std::endl;
    {
      auto mysql_data_store = MySqlProfileDataStore(
        mysql_config.m_address, mysql_config.m_schema, mysql_config.m_username,
        mysql_config.m_password);
      auto data_store = BufferedProfileDataStore(
        &mysql_data_store, profile_config.m_buffer_size);
      profile_writes(data_store, profile_config);
    }
    {
      auto mysql_data_store = MySqlProfileDataStore(
        mysql_config.m_address, mysql_config.m_schema, mysql_config.m_username,
        mysql_config.m_password);
      auto data_store = BufferedProfileDataStore(
        &mysql_data_store, profile_config.m_buffer_size);
      profile_reads(data_store, profile_config);
    }
  }

  void profile_async_data_store(
      const MySqlConfig& mysql_config, const ProfileConfig& profile_config) {
    std::cout << "AsyncDataStore" << std::endl;
    {
      auto mysql_data_store = MySqlProfileDataStore(
        mysql_config.m_address, mysql_config.m_schema, mysql_config.m_username,
        mysql_config.m_password);
      auto data_store = AsyncProfileDataStore(&mysql_data_store);
      profile_writes(data_store, profile_config);
    }
    {
      auto mysql_data_store = MySqlProfileDataStore(
        mysql_config.m_address, mysql_config.m_schema, mysql_config.m_username,
        mysql_config.m_password);
      auto data_store = AsyncProfileDataStore(&mysql_data_store);
      profile_reads(data_store, profile_config);
    }
  }
}

int main(int argc, const char** argv) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" DATA_STORE_PROFILER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto profile_config = ProfileConfig::parse(config);
    auto mysql_config = try_or_nest([&] {
      return MySqlConfig::parse(get_node(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    profile_buffered_data_store(mysql_config, profile_config);
    profile_async_data_store(mysql_config, profile_config);
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
