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
using namespace TCLAP;

namespace {
  std::vector<std::string> Tokenize(const std::string& input) {
    auto result = std::vector<std::string>();
    split(result, input, is_any_of(" "));
    return result;
  }

  DirectoryEntry LoadAccount(
      ApplicationServiceLocatorClient& serviceLocatorClient,
      const std::string& name) {
    if(name.empty()) {
      throw std::runtime_error("No name specified.");
    } else if(name[0] == '@') {
      return serviceLocatorClient->LoadDirectoryEntry(
        lexical_cast<int>(name.substr(1)));
    }
    auto account = serviceLocatorClient->FindAccount(name);
    if(account.is_initialized()) {
      return *account;
    }
    throw std::runtime_error("Account not found.");
  }

  DirectoryEntry LoadPath(ApplicationServiceLocatorClient& serviceLocatorClient,
      const DirectoryEntry& currentDirectory, const std::string& path) {
    if(path.empty()) {
      throw std::runtime_error("No path specified.");
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
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" ADMIN_CLIENT_VERSION
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
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(config);
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    std::cout << "Connecting to the service locator: ";
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
    return -1;
  }
  std::cout << "Passed." << std::endl;
  auto account = DirectoryEntry();
  auto currentDirectory = DirectoryEntry();
  auto children = std::vector<DirectoryEntry>();
  auto parents = std::vector<DirectoryEntry>();
  try {
    account = serviceLocatorClient->GetAccount();
    currentDirectory = serviceLocatorClient->LoadParents(account).front();
    children = serviceLocatorClient->LoadChildren(currentDirectory);
    parents = serviceLocatorClient->LoadParents(currentDirectory);
  } catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
    return -1;
  }
  std::cout << "Passed." << std::endl;
  while(!ReceivedKillEvent()) {
    try {
      std::cout << ">>> ";
      auto input = std::string();
      std::getline(std::cin, input);
      auto tokens = Tokenize(input);
      if(tokens[0] == "mkacc") {
        serviceLocatorClient->MakeAccount(tokens.at(1), tokens.at(2),
          currentDirectory);
      } else if(tokens[0] == "password") {
        auto source = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        if(source.m_type != DirectoryEntry::Type::ACCOUNT) {
          throw std::runtime_error("Not an account.");
        }
        serviceLocatorClient->StorePassword(source, tokens.at(2));
      } else if(tokens[0] == "mkdir") {
        serviceLocatorClient->MakeDirectory(tokens.at(1), currentDirectory);
      } else if(tokens[0] == "chmod") {
        auto source = LoadAccount(serviceLocatorClient, tokens.at(1));
        auto target = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(2));
        auto permissions = Permissions::FromRepresentation(
          lexical_cast<int>(tokens.at(3)));
        serviceLocatorClient->StorePermissions(source, target, permissions);
      } else if(tokens[0] == "associate") {
        auto entry = LoadAccount(serviceLocatorClient, tokens.at(1));
        serviceLocatorClient->Associate(entry, currentDirectory);
      } else if(tokens[0] == "detach") {
        auto child = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        serviceLocatorClient->Detach(child, currentDirectory);
      } else if(tokens[0] == "cd") {
        auto newDirectory = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        if(newDirectory.m_type != DirectoryEntry::Type::DIRECTORY) {
          throw std::runtime_error("Not a directory.");
        }
        currentDirectory = newDirectory;
        parents = serviceLocatorClient->LoadParents(currentDirectory);
        children = serviceLocatorClient->LoadChildren(currentDirectory);
      } else if(tokens[0] == "lch") {
        children = serviceLocatorClient->LoadChildren(currentDirectory);
        for(auto& child : children) {
          auto type = [&] {
            if(child.m_type == DirectoryEntry::Type::DIRECTORY) {
              return "<DIR>";
            } else {
              return "";
            }
          }();
          std::cout << "\t" << type << "\t" << child.m_id << "\t" <<
            child.m_name << std::endl;
        }
      } else if(tokens[0] == "lpr") {
        parents = serviceLocatorClient->LoadParents(currentDirectory);
        for(auto& parent : parents) {
          auto type = [&] {
            if(parent.m_type == DirectoryEntry::Type::DIRECTORY) {
              return "<DIR>";
            } else {
              return "";
            }
          }();
          std::cout << "\t" << type << "\t" << parent.m_id << "\t" <<
            parent.m_name << std::endl;
        }
      } else if(tokens[0] == "del") {
        auto path = LoadPath(serviceLocatorClient, currentDirectory,
          tokens.at(1));
        serviceLocatorClient->Delete(path);
      } else if(tokens[0] == "locate") {
        for(auto& service : serviceLocatorClient->Locate(tokens.at(1))) {
          std::cout << service.GetName() << " " << service.GetId() << "\n";
          std::cout << service.GetAccount().m_name << " " <<
            service.GetAccount().m_id << "\n";
          service.GetProperties().Save(std::cout);
          std::cout << std::endl;
        }
      } else if(tokens[0] == "exit") {
        break;
      } else {
        std::cout << "Unknown command." << std::endl;
      }
    } catch(const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }
}
