#include <cmath>
#include <fstream>
#include <iostream>
#include <tclap/CmdLine.h>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/optional/optional.hpp>
#include "DataStoreProfiler/MySqlDataStore.hpp"
#include "DataStoreProfiler/Version.hpp"

using namespace Beam;
using namespace Beam::MySql;
using namespace boost;
using namespace std;
using namespace TCLAP;

namespace {
  struct ProfileConfig {
    int m_indexCount;
    int m_seedCount;
    int m_growthFactor;
  };

  ProfileConfig ParseProfileConfig(const YAML::Node& config) {
    ProfileConfig profileConfig;
    profileConfig.m_indexCount = Extract<int>(config, "index_count");
    profileConfig.m_seedCount = Extract<int>(config, "seed_count");
    profileConfig.m_growthFactor = Extract<int>(config, "growth_factor");
    return profileConfig;
  }

  template<typename DataStore>
  void ProfileWrites(DataStore& dataStore, const ProfileConfig& config) {
    auto groups = std::ceil(std::log2(
      static_cast<double>(config.m_indexCount) / config.m_seedCount));
    auto range = std::pow(2, groups);
    auto index = std::rand() % static_cast<int>(groups);
    std::cout << groups << " " << range << " " << index << std::endl;
  }
}

int main(int argc, const char** argv) {
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
//  mysqlDataStore.Open();
//  mysqlDataStore.Clear();
  ProfileWrites(mysqlDataStore, profileConfig);
  return 0;
}
