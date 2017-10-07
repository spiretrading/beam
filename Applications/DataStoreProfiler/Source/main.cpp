#include <cmath>
#include <fstream>
#include <iostream>
#include <tclap/CmdLine.h>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Threading/ThreadPool.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/optional/optional.hpp>
#include "DataStoreProfiler/BufferedDataStore.hpp"
#include "DataStoreProfiler/SessionCachedDataStore.hpp"
#include "DataStoreProfiler/Entry.hpp"
#include "DataStoreProfiler/MySqlDataStore.hpp"
#include "DataStoreProfiler/Version.hpp"

using namespace Beam;
using namespace Beam::MySql;
using namespace Beam::Queries;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;
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
    ProfileConfig profileConfig;
    profileConfig.m_indexCount = Extract<int>(config, "index_count");
    profileConfig.m_seedCount = Extract<int>(config, "seed_count");
    profileConfig.m_growthFactor = Extract<int>(config, "growth_factor");
    profileConfig.m_bufferSize = Extract<std::size_t>(config, "buffer_size");
    profileConfig.m_iterations = Extract<int>(config, "iterations");
    profileConfig.m_startTime = Extract<ptime>(config, "start_time");
    profileConfig.m_timeStep = Extract<time_duration>(config, "time_step");
    for(auto i = 0; i < profileConfig.m_indexCount; ++i) {
      std::string name;
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
    dataStore.Open();
    dataStore.Clear();
    auto start = boost::posix_time::microsec_clock::universal_time();
    int groups = static_cast<int>(std::ceil(std::log2(
      static_cast<double>(config.m_indexCount) / config.m_seedCount)));
    int range = static_cast<int>(std::pow(2, groups));
    auto timestamp = config.m_startTime;
    unordered_map<std::string, Queries::Sequence> sequences;
    for(auto i = 0; i < config.m_iterations; ++i) {
      auto group = std::abs(
        static_cast<int>(std::log2(1 + (std::rand() % range))) - (groups - 1));
      auto lowerIndex = config.m_seedCount *
        static_cast<int>(std::pow(2, group) - 1);
      auto groupSize = config.m_seedCount * static_cast<int>(
        std::pow(2, group));
      auto index = lowerIndex + (rand() % groupSize);
      if(index > config.m_indexCount) {
        index = lowerIndex + (index - config.m_indexCount);
      }
      Entry entry{config.m_names[index], rand() % 100, rand() % 10000000,
        rand() % 10000000, "dummy", timestamp};
      auto& sequence = sequences[entry.m_name];
      sequence = Increment(sequence);
      auto sequencedIndexedEntry = MakeSequencedValue(
        MakeIndexedValue(entry, entry.m_name), sequence);
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
    dataStore.Open();
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
      EntryQuery query;
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
}

int main(int argc, const char** argv) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" DATA_STORE_PROFILER_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  ThreadPool threadPool;
  ProfileConfig profileConfig;
  try {
    profileConfig = ParseProfileConfig(config);
  } catch(const  std::exception& e) {
    cerr << "Unable to parse config: " << e.what() << endl;
    return -1;
  }
  MySqlConfig mySqlConfig;
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  MySqlDataStore mysqlDataStore{mySqlConfig.m_address, mySqlConfig.m_schema,
    mySqlConfig.m_username, mySqlConfig.m_password};
  Beam::BufferedDataStore<MySqlDataStore*> bufferedDataStore{&mysqlDataStore,
    profileConfig.m_bufferSize, Ref(threadPool)};
  Beam::SessionCachedDataStore<Beam::BufferedDataStore<MySqlDataStore*>*>
    sessionCachedDataStore{&bufferedDataStore, 1000000};
  ProfileWrites(bufferedDataStore, profileConfig);
  ProfileReads(bufferedDataStore, profileConfig);
  return 0;
}
