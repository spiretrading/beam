#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace std;
using namespace TCLAP;

namespace {
  vector<string> Tokenize(const string& input) {
    vector<string> result;
    split(result, input, is_any_of(" "));
    return result;
  }

  DirectoryEntry LoadAccount(
      ApplicationServiceLocatorClient& serviceLocatorClient,
      const string& name) {
    if(name.empty()) {
      throw runtime_error("No name specified.");
    } else if(name[0] == '@') {
      return serviceLocatorClient->LoadDirectoryEntry(
        lexical_cast<int>(name.substr(1)));
    }
    auto account = serviceLocatorClient->FindAccount(name);
    if(account.is_initialized()) {
      return *account;
    }
    throw runtime_error("Account not found.");
  }

  DirectoryEntry LoadPath(ApplicationServiceLocatorClient& serviceLocatorClient,
      const DirectoryEntry& currentDirectory, const string& path) {
    if(path.empty()) {
      throw runtime_error("No path specified.");
    } else if(path[0] == '/') {
      return serviceLocatorClient->LoadDirectoryEntry(
        DirectoryEntry::GetStarDirectory(), path.substr(1));
    } else if(path[0] == '@') {
      return serviceLocatorClient->LoadDirectoryEntry(
        lexical_cast<int>(path.substr(1)));
    }
    return serviceLocatorClient->LoadDirectoryEntry(currentDirectory, path);
  }
}

int main(int argc, char** argv) {
  string configFile;
  try {
    CmdLine cmd("", ' ', "1.0-r" ADMIN_CLIENT_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd.");
    ValueArg<string> configArg("c", "config", "Configuration file", false,
      "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(config);
  } catch(const std::exception& e) {
    cerr << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ApplicationServiceLocatorClient serviceLocatorClient;
  try {
    cout << "Connecting to the service locator: ";
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(const std::exception& e) {
    cout << e.what() << endl;
    return -1;
  }
  cout << "Passed." << endl;
  DirectoryEntry account;
  DirectoryEntry currentDirectory;
  vector<DirectoryEntry> children;
  vector<DirectoryEntry> parents;
  try {
    account = serviceLocatorClient->GetAccount();
    currentDirectory = serviceLocatorClient->LoadParents(account).front();
    children = serviceLocatorClient->LoadChildren(currentDirectory);
    parents = serviceLocatorClient->LoadParents(currentDirectory);
  } catch(const std::exception& e) {
    cout << e.what() << endl;
    return -1;
  }
  cout << "Passed." << endl;
  while(!ReceivedKillEvent()) {
    try {
      cout << ">>> ";
      string input;
      getline(cin, input);
      vector<string> tokens(Tokenize(input));
      if(tokens[0] == "mkacc") {
        serviceLocatorClient->MakeAccount(tokens.at(1), tokens.at(2),
          currentDirectory);
      } else if(tokens[0] == "password") {
        DirectoryEntry source = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        if(source.m_type != DirectoryEntry::Type::ACCOUNT) {
          throw runtime_error("Not an account.");
        }
        serviceLocatorClient->StorePassword(source, tokens.at(2));
      } else if(tokens[0] == "mkdir") {
        serviceLocatorClient->MakeDirectory(tokens.at(1), currentDirectory);
      } else if(tokens[0] == "chmod") {
        DirectoryEntry source = LoadAccount(serviceLocatorClient, tokens.at(1));
        DirectoryEntry target = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(2));
        Permissions permissions = Permissions::FromRepresentation(
          lexical_cast<int>(tokens.at(3)));
        serviceLocatorClient->StorePermissions(source, target, permissions);
      } else if(tokens[0] == "associate") {
        DirectoryEntry entry = LoadAccount(serviceLocatorClient, tokens.at(1));
        serviceLocatorClient->Associate(entry, currentDirectory);
      } else if(tokens[0] == "detach") {
        DirectoryEntry child = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        serviceLocatorClient->Detach(child, currentDirectory);
      } else if(tokens[0] == "cd") {
        DirectoryEntry newDirectory = LoadPath(serviceLocatorClient,
          currentDirectory, tokens.at(1));
        if(newDirectory.m_type != DirectoryEntry::Type::DIRECTORY) {
          throw runtime_error("Not a directory.");
        }
        currentDirectory = newDirectory;
        parents = serviceLocatorClient->LoadParents(currentDirectory);
        children = serviceLocatorClient->LoadChildren(currentDirectory);
      } else if(tokens[0] == "lch") {
        children = serviceLocatorClient->LoadChildren(currentDirectory);
        for(auto i = children.begin(); i != children.end(); ++i) {
          string type;
          if(i->m_type == DirectoryEntry::Type::DIRECTORY) {
            type = "<DIR>";
          } else {
            type = "";
          }
          cout << "\t" << type << "\t" << i->m_id << "\t" << i->m_name << endl;
        }
      } else if(tokens[0] == "lpr") {
        parents = serviceLocatorClient->LoadParents(currentDirectory);
        for(auto i = parents.begin(); i != parents.end(); ++i) {
          string type;
          if(i->m_type == DirectoryEntry::Type::DIRECTORY) {
            type = "<DIR>";
          } else {
            type = "";
          }
          cout << "\t" << type << "\t" << i->m_id << "\t" << i->m_name << endl;
        }
      } else if(tokens[0] == "del") {
        DirectoryEntry path = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        serviceLocatorClient->Delete(path);
      } else if(tokens[0] == "locate") {
        vector<ServiceEntry> services = serviceLocatorClient->Locate(
          tokens.at(1));
        for(vector<ServiceEntry>::const_iterator i = services.begin();
            i != services.end(); ++i) {
          cout << i->GetName() << " " << i->GetId() << "\n";
          cout << i->GetAccount().m_name << " " << i->GetAccount().m_id << "\n";
          i->GetProperties().Save(cout);
          cout << endl;
        }
      } else if(tokens[0] == "exit") {
        break;
      } else {
        cout << "Unknown command." << endl;
      }
    } catch(const std::exception& e) {
      cout << e.what() << endl;
    }
  }
}
