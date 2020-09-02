#include <cmath>
#include <fstream>
#include <iostream>
#include <tclap/CmdLine.h>
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
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace TCLAP;

namespace {
  struct ProfileConfig {
    int m_indexCount;
    int m_seedCount;
    int m_growthFactor;
    std::size_t m_bufferSize;
    int m_iterations;
    boost::posix_time::ptime m_startTime;
    boost::posix_time::time_duration m_timeStep;
    std::vector<std::string> m_names;
  };

  ProfileConfig ParseProfileConfig(const YAML::Node& config) {
    auto profileConfig = ProfileConfig();
    profileConfig.m_indexCount = Extract<int>(config, "index_count");
    profileConfig.m_seedCount = Extract<int>(config, "seed_count");
    profileConfig.m_growthFactor = Extract<int>(config, "growth_factor");
    profileConfig.m_bufferSize = Extract<std::size_t>(config, "buffer_size");
    profileConfig.m_iterations = Extract<int>(config, "iterations");
    profileConfig.m_startTime = Extract<ptime>(config, "start_time");
    profileConfig.m_timeStep = Extract<time_duration>(config, "time_step");
    for(auto i = 0; i < profileConfig.m_indexCount; ++i) {
      auto name = std::string();
      do {
        name.clear();
        for(auto j = 0; j < 4; ++j) {
          name += 'A' + static_cast<char>(rand() % 26);
        }
      } while(std::find(profileConfig.m_names.begin(),
        profileConfig.m_names.end(), name) != profileConfig.m_names.end());
      profileConfig.m_names.push_back(name);
    }
    return profileConfig;
  }

  template<typename DataStore>
  void ProfileWrites(DataStore& dataStore, const ProfileConfig& config) {
    dataStore.Clear();
    auto start = boost::posix_time::microsec_clock::universal_time();
    auto groups = static_cast<int>(std::ceil(std::log2(
      static_cast<double>(config.m_indexCount) / config.m_seedCount)));
    auto range = static_cast<int>(std::pow(2, groups));
    auto timestamp = config.m_startTime;
    auto sequences = std::unordered_map<std::string, Queries::Sequence>();
    for(auto i = 0; i < config.m_iterations; ++i) {
      auto group = std::abs(
        static_cast<int>(std::log2(1 + (std::rand() % range))) - (groups - 1));
      auto lowerIndex = config.m_seedCount *
        static_cast<int>(std::pow(2, group) - 1);
      auto groupSize = config.m_seedCount * static_cast<int>(
        std::pow(2, group));
      auto index = lowerIndex + (rand() % groupSize);
      if(index >= config.m_indexCount) {
        index = lowerIndex + (index - config.m_indexCount);
      }
      auto entry = Entry{config.m_names[index], rand() % 100, rand() % 10000000,
        rand() % 10000000, "dummy", timestamp};
      auto& sequence = sequences[entry.m_name];
      sequence = Increment(sequence);
      auto sequencedIndexedEntry = SequencedValue(IndexedValue(entry,
        entry.m_name), sequence);
      dataStore.Store(sequencedIndexedEntry);
      timestamp += config.m_timeStep;
    }
    dataStore.Close();
    auto end = boost::posix_time::microsec_clock::universal_time();
    auto elapsed = end - start;
    auto rate = config.m_iterations / elapsed.total_seconds();
    std::cout << "ProfileWrites: " << (end - start) << " " << rate << std::endl;
  }

  template<typename DataStore>
  void ProfileReads(DataStore& dataStore, const ProfileConfig& config) {
    auto endTimestamp = config.m_startTime +
      config.m_timeStep * config.m_iterations;
    auto start = boost::posix_time::microsec_clock::universal_time();
    auto count = 0;
    for(auto i = 0; i < config.m_iterations; ++i) {
      const auto& name = config.m_names[rand() % config.m_indexCount];
      auto startTime = config.m_startTime + milliseconds(rand() %
        (endTimestamp - config.m_startTime).total_milliseconds());
      auto endTime = startTime + milliseconds(rand() %
        (endTimestamp - startTime).total_milliseconds());
      auto query = EntryQuery();
      query.SetIndex(name);
      query.SetRange(startTime, endTime);
      query.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto result = dataStore.LoadEntries(query);
      count += result.size();
    }
    dataStore.Close();
    auto end = boost::posix_time::microsec_clock::universal_time();
    auto elapsed = end - start;
    auto rate = config.m_iterations / elapsed.total_seconds();
    std::cout << "ProfileReads: " << (end - start) << " " << rate << std::endl;
  }

  void ProfileBufferedDataStore(const MySqlConfig& mySqlConfig,
      const ProfileConfig& profileConfig) {
    std::cout << "BufferedDataStore" << std::endl;
    {
      auto mysqlDataStore = MySqlDataStore(mySqlConfig.m_address,
        mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password);
      auto dataStore = Beam::BufferedDataStore(&mysqlDataStore,
        profileConfig.m_bufferSize);
      ProfileWrites(dataStore, profileConfig);
    }
    {
      auto mysqlDataStore = MySqlDataStore(mySqlConfig.m_address,
        mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password);
      auto dataStore = Beam::BufferedDataStore(&mysqlDataStore,
        profileConfig.m_bufferSize);
      ProfileReads(dataStore, profileConfig);
    }
  }

  void ProfileAsyncDataStore(const MySqlConfig& mySqlConfig,
      const ProfileConfig& profileConfig) {
    std::cout << "AsyncDataStore" << std::endl;
    {
      auto mysqlDataStore = MySqlDataStore(mySqlConfig.m_address,
        mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password);
      auto dataStore = Beam::AsyncDataStore(&mysqlDataStore);
      ProfileWrites(dataStore, profileConfig);
    }
    {
      auto mysqlDataStore = MySqlDataStore(mySqlConfig.m_address,
        mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password);
      auto dataStore = Beam::AsyncDataStore(&mysqlDataStore);
      ProfileReads(dataStore, profileConfig);
    }
  }
}

int main(int argc, const char** argv) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" DATA_STORE_PROFILER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto profileConfig = ProfileConfig();
  try {
    profileConfig = ParseProfileConfig(config);
  } catch(const  std::exception& e) {
    std::cerr << "Unable to parse config: " << e.what() << std::endl;
    return -1;
  }
  auto mySqlConfig = MySqlConfig();
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
  ProfileBufferedDataStore(mySqlConfig, profileConfig);
  ProfileAsyncDataStore(mySqlConfig, profileConfig);
  return 0;
}
