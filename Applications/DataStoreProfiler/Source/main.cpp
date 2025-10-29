#include <cmath>
#include <iostream>
#include <boost/optional/optional.hpp>
#include "Beam/Sql/MySqlConfig.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "DataStoreProfiler/AsyncDataStore.hpp"
#include "DataStoreProfiler/BufferedDataStore.hpp"
#include "DataStoreProfiler/Entry.hpp"
#include "DataStoreProfiler/MySqlDataStore.hpp"
#include "Version.hpp"

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
