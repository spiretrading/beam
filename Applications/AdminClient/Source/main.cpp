#include <iostream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;

namespace {
  std::vector<std::string> tokenize(const std::string& input) {
    auto result = std::vector<std::string>();
    split(result, input, is_any_of(" "));
    return result;
  }

  DirectoryEntry load_account(
      ApplicationServiceLocatorClient& service_locator_client,
      const std::string& name) {
    if(name.empty()) {
      throw std::runtime_error("No name specified.");
    } else if(name[0] == '@') {
      return service_locator_client.load_directory_entry(
        lexical_cast<int>(name.substr(1)));
    }
    if(auto account = service_locator_client.find_account(name)) {
      return *account;
    }
    throw std::runtime_error("Account not found.");
  }

  DirectoryEntry load_path(
      ApplicationServiceLocatorClient& service_locator_client,
      const DirectoryEntry& current_directory, const std::string& path) {
    if(path.empty()) {
      throw std::runtime_error("No path specified.");
    } else if(path[0] == '/') {
      return service_locator_client.load_directory_entry(
        DirectoryEntry::STAR_DIRECTORY, path.substr(1));
    } else if(path[0] == '@') {
      return service_locator_client.load_directory_entry(
        lexical_cast<int>(path.substr(1)));
    }
    return service_locator_client.load_directory_entry(
      current_directory, path);
  }

  void print_help() {
    std::cout << "Available commands:\n\n";
    std::cout << "  mkacc <name> <password>\n";
    std::cout << "    Create a new account in the current directory\n\n";
    std::cout << "  password <path> <password>\n";
    std::cout << "    Set the password for an account\n\n";
    std::cout << "  mkdir <name>\n";
    std::cout << "    Create a new directory in the current directory\n\n";
    std::cout << "  chmod <account> <path> <permissions>\n";
    std::cout << "    Set permissions for an account on a path\n\n";
    std::cout << "  associate <account>\n";
    std::cout << "    Associate an account with the current directory\n\n";
    std::cout << "  detach <path>\n";
    std::cout << "    Detach a child from the current directory\n\n";
    std::cout << "  cd <path>\n";
    std::cout << "    Change to a different directory\n\n";
    std::cout << "  lch\n";
    std::cout << "    List children of the current directory\n\n";
    std::cout << "  lpr\n";
    std::cout << "    List parents of the current directory\n\n";
    std::cout << "  del <path>\n";
    std::cout << "    Delete a directory entry\n\n";
    std::cout << "  locate <service>\n";
    std::cout << "    Locate a service by name\n\n";
    std::cout << "  help\n";
    std::cout << "    Display this help message\n\n";
    std::cout << "  exit\n";
    std::cout << "    Exit the application\n" << std::endl;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ServiceLocatorClientConfig::parse(
      parse_command_line(argc, argv, "1.0-r" ADMIN_CLIENT_VERSION
        "\nCopyright (C) 2026 Spire Trading Inc."));
    auto service_locator_client = ApplicationServiceLocatorClient(config);
    auto account = service_locator_client.get_account();
    auto current_directory = try_or_nest([&] {
      return service_locator_client.load_parents(account).front();
    }, std::runtime_error("Unable to load home directory."));
    auto children = try_or_nest([&] {
      return service_locator_client.load_children(current_directory);
    }, std::runtime_error("Unable to load the home directory's children."));
    auto parents = try_or_nest([&] {
      return service_locator_client.load_parents(current_directory);
    }, std::runtime_error("Unable to load the home directory's parents."));
    while(!received_kill_event()) {
      std::cout << ">>> ";
      auto input = std::string();
      std::getline(std::cin, input);
      auto tokens = tokenize(input);
      if(tokens.empty()) {
        continue;
      } else if(tokens[0] == "mkacc") {
        service_locator_client.make_account(
          tokens.at(1), tokens.at(2), current_directory);
      } else if(tokens[0] == "password") {
        auto source =
          load_path(service_locator_client, current_directory, tokens.at(1));
        if(source.m_type != DirectoryEntry::Type::ACCOUNT) {
          throw std::runtime_error("Not an account.");
        }
        service_locator_client.store_password(source, tokens.at(2));
      } else if(tokens[0] == "mkdir") {
        service_locator_client.make_directory(tokens.at(1), current_directory);
      } else if(tokens[0] == "chmod") {
        auto source = load_account(service_locator_client, tokens.at(1));
        auto target =
          load_path(service_locator_client, current_directory, tokens.at(2));
        auto permissions =
          Permissions::from_bitmask(lexical_cast<int>(tokens.at(3)));
        service_locator_client.store(source, target, permissions);
      } else if(tokens[0] == "associate") {
        auto entry = load_account(service_locator_client, tokens.at(1));
        service_locator_client.associate(entry, current_directory);
      } else if(tokens[0] == "detach") {
        auto child =
          load_path(service_locator_client, current_directory, tokens.at(1));
        service_locator_client.detach(child, current_directory);
      } else if(tokens[0] == "cd") {
        auto new_directory =
          load_path(service_locator_client, current_directory, tokens.at(1));
        if(new_directory.m_type != DirectoryEntry::Type::DIRECTORY) {
          throw std::runtime_error("Not a directory.");
        }
        current_directory = new_directory;
        parents = service_locator_client.load_parents(current_directory);
        children = service_locator_client.load_children(current_directory);
      } else if(tokens[0] == "lch") {
        children = service_locator_client.load_children(current_directory);
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
        parents = service_locator_client.load_parents(current_directory);
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
        auto path =
          load_path(service_locator_client, current_directory, tokens.at(1));
        service_locator_client.remove(path);
      } else if(tokens[0] == "locate") {
        for(auto& service : service_locator_client.locate(tokens.at(1))) {
          std::cout << service.get_name() << " " << service.get_id() << "\n";
          std::cout << service.get_account().m_name << " " <<
            service.get_account().m_id << "\n";
          service.get_properties().save(std::cout);
          std::cout << std::endl;
        }
      } else if(tokens[0] == "help") {
        print_help();
      } else if(tokens[0] == "exit") {
        break;
      } else {
        std::cout << "Unknown command." << std::endl;
      }
    }
  } catch(const std::exception&) {
    report_current_exception();
    return -1;
  }
  return 0;
}
