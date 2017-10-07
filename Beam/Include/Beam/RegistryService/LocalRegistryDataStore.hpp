#ifndef BEAM_LOCALREGISTRYDATASTORE_HPP
#define BEAM_LOCALREGISTRYDATASTORE_HPP
#include <unordered_map>
#include <boost/throw_exception.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/RegistryService/RegistryDataStore.hpp"
#include "Beam/RegistryService/RegistryDataStoreException.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class LocalRegistryDataStore
      \brief Implements the RegistryDataStore using local memory.
   */
  class LocalRegistryDataStore : public RegistryDataStore {
    public:

      //! Constructs a LocalRegistryDataStore.
      LocalRegistryDataStore();

      virtual ~LocalRegistryDataStore() override;

      virtual RegistryEntry LoadParent(
        const RegistryEntry& registryEntry) override;

      virtual std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& directory) override;

      virtual RegistryEntry LoadRegistryEntry(std::uint64_t id) override;

      virtual RegistryEntry Copy(const RegistryEntry& source,
        const RegistryEntry& destination) override;

      virtual void Move(const RegistryEntry& source,
        const RegistryEntry& destination) override;

      virtual void Delete(const RegistryEntry& registryEntry) override;

      virtual IO::SharedBuffer Load(
        const RegistryEntry& registryEntry) override;

      virtual RegistryEntry Store(const RegistryEntry& registryEntry,
        const IO::SharedBuffer& value) override;

      virtual void WithTransaction(
        const std::function<void ()>& transaction) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::uint64_t m_nextId;
      std::unordered_map<std::uint64_t, RegistryEntry> m_entries;
      std::unordered_map<std::uint64_t, std::uint64_t> m_parents;
      std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> m_children;
      std::unordered_map<std::uint64_t, IO::SharedBuffer> m_values;
      IO::OpenState m_openState;
  };

  inline LocalRegistryDataStore::LocalRegistryDataStore()
      : m_nextId{1} {
    auto root = RegistryEntry::GetRoot();
    m_entries.insert(std::make_pair(root.m_id, root));
    m_parents.insert(std::make_pair(root.m_id, root.m_id));
    m_children.insert(std::make_pair(root.m_id, std::vector<std::uint64_t>()));
  }

  inline LocalRegistryDataStore::~LocalRegistryDataStore() {
    Close();
  }

  inline RegistryEntry LocalRegistryDataStore::LoadParent(
      const RegistryEntry& registryEntry) {
    return m_entries.at(m_parents.at(registryEntry.m_id));
  }

  inline std::vector<RegistryEntry> LocalRegistryDataStore::LoadChildren(
      const RegistryEntry& directory) {
    std::vector<RegistryEntry> children;
    auto childrenIterator = m_children.find(directory.m_id);
    if(childrenIterator == m_children.end()) {
      return children;
    }
    auto& entries = childrenIterator->second;
    std::transform(entries.begin(), entries.end(), std::back_inserter(children),
      [&] (auto id) {
        return m_entries.at(id);
      });
    return children;
  }

  inline RegistryEntry LocalRegistryDataStore::LoadRegistryEntry(
      std::uint64_t id) {
    auto entryIterator = m_entries.find(id);
    if(entryIterator == m_entries.end()) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Registry entry not found."});
    }
    return entryIterator->second;
  }

  inline RegistryEntry LocalRegistryDataStore::Copy(const RegistryEntry& source,
      const RegistryEntry& destination) {
    auto newEntry = source;
    auto& children = m_children.at(destination.m_id);
    auto entryIterator = std::find_if(children.begin(), children.end(),
      [=] (auto entry) {
        return m_entries.at(entry).m_name == source.m_name;
      });
    if(entryIterator != children.end()) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Registry entry already exists."});
    }
    newEntry.m_id = m_nextId;
    ++m_nextId;
    m_entries.insert(std::make_pair(newEntry.m_id, newEntry));
    m_parents.insert(std::make_pair(newEntry.m_id, destination.m_id));
    children.push_back(newEntry.m_id);
    if(newEntry.m_type == RegistryEntry::Type::DIRECTORY) {
      m_children.insert(std::make_pair(newEntry.m_id,
        std::vector<std::uint64_t>()));
    } else {
      auto valueIterator = m_values.find(source.m_id);
      IO::SharedBuffer value;
      if(valueIterator != m_values.end()) {
        value = valueIterator->second;
      }
      m_values.insert(std::make_pair(newEntry.m_id, value));
    }
    return newEntry;
  }

  inline void LocalRegistryDataStore::Move(const RegistryEntry& source,
      const RegistryEntry& destination) {
    auto& destinationChildren = m_children.at(destination.m_id);
    auto entryIterator = std::find_if(destinationChildren.begin(),
      destinationChildren.end(),
      [=] (auto entry) {
        return m_entries.at(entry).m_name == source.m_name;
      });
    if(entryIterator != destinationChildren.end()) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Registry entry already exists."});
    }
    auto& sourceChildren = m_children.at(m_parents.at(source.m_id));
    sourceChildren.erase(std::find(sourceChildren.begin(), sourceChildren.end(),
      source.m_id));
    m_parents[source.m_id] = destination.m_id;
    destinationChildren.push_back(source.m_id);
  }

  inline void LocalRegistryDataStore::Delete(
      const RegistryEntry& registryEntry) {
    m_entries.erase(registryEntry.m_id);
    auto& children = m_children.at(m_parents.at(registryEntry.m_id));
    children.erase(std::find(children.begin(), children.end(),
      registryEntry.m_id));
    m_parents.erase(registryEntry.m_id);
    m_values.erase(registryEntry.m_id);
  }

  inline IO::SharedBuffer LocalRegistryDataStore::Load(
      const RegistryEntry& registryEntry) {
    auto valueIterator = m_values.find(registryEntry.m_id);
    if(valueIterator == m_values.end()) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{"Entry not found."});
    }
    return valueIterator->second;
  }

  inline RegistryEntry LocalRegistryDataStore::Store(
      const RegistryEntry& registryEntry, const IO::SharedBuffer& value) {
    if(registryEntry.m_type != RegistryEntry::Type::VALUE) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Entry does not represent a value."});
    }
    auto& entry = m_entries.at(registryEntry.m_id);
    ++entry.m_version;
    m_values[entry.m_id] = value;
    return entry;
  }

  inline void LocalRegistryDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    transaction();
  }

  inline void LocalRegistryDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalRegistryDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }
}
}

#endif
