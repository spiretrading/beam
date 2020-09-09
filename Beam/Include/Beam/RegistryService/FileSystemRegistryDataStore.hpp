#ifndef BEAM_FILE_SYSTEM_REGISTRY_DATA_STORE_HPP
#define BEAM_FILE_SYSTEM_REGISTRY_DATA_STORE_HPP
#include <filesystem>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/BasicIStreamReader.hpp"
#include "Beam/IO/BasicOStreamWriter.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/RegistryService/FileSystemRegistryDataStoreDetails.hpp"
#include "Beam/RegistryService/RegistryDataStore.hpp"
#include "Beam/RegistryService/RegistryDataStoreException.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::RegistryService {

  /** Implements the RegistryDataStore using the local file system. */
  class FileSystemRegistryDataStore : public RegistryDataStore {
    public:

      /**
       * Constructs a FileSystemRegistryDataStore.
       * @param root The directory storing the files.
       */
      FileSystemRegistryDataStore(const std::filesystem::path& root);

      ~FileSystemRegistryDataStore() override;

      RegistryEntry LoadParent(const RegistryEntry& registryEntry) override;

      std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& directory) override;

      RegistryEntry LoadRegistryEntry(std::uint64_t id) override;

      RegistryEntry Copy(const RegistryEntry& source,
        const RegistryEntry& destination) override;

      void Move(const RegistryEntry& source,
        const RegistryEntry& destination) override;

      void Delete(const RegistryEntry& registryEntry) override;

      IO::SharedBuffer Load(const RegistryEntry& registryEntry) override;

      RegistryEntry Store(const RegistryEntry& registryEntry,
        const IO::SharedBuffer& value) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::filesystem::path m_root;
      std::uint64_t m_nextId;
      IO::OpenState m_openState;

      std::filesystem::path GetPath(std::uint64_t id);
      Details::RegistryEntryRecord LoadRecord(std::uint64_t id);
      void SaveRecord(const Details::RegistryEntryRecord& record);
      std::uint64_t LoadNextEntryId();
  };

  inline FileSystemRegistryDataStore::FileSystemRegistryDataStore(
      const std::filesystem::path& root)
      : m_root(root) {
    std::filesystem::create_directories(m_root);
    if(!std::filesystem::exists(m_root / "settings.dat")) {
      auto writer = IO::BasicOStreamWriter<std::ofstream>(
        Initialize(m_root / "settings.dat", std::ios::binary));
      auto buffer = IO::SharedBuffer();
      auto sender = Serialization::BinarySender<IO::SharedBuffer>();
      sender.SetSink(Ref(buffer));
      sender.Send(std::uint64_t(1));
      writer.Write(buffer);
    }
    try {
      auto root = LoadRegistryEntry(RegistryEntry::GetRoot().m_id);
    } catch(const std::exception&) {
      auto rootRecord = Details::RegistryEntryRecord();
      rootRecord.m_registryEntry = RegistryEntry::GetRoot();
      rootRecord.m_parent = 0;
      SaveRecord(rootRecord);
    }
  }

  inline FileSystemRegistryDataStore::~FileSystemRegistryDataStore() {
    Close();
  }

  inline RegistryEntry FileSystemRegistryDataStore::LoadParent(
      const RegistryEntry& registryEntry) {
    auto record = LoadRecord(registryEntry.m_id);
    return LoadRegistryEntry(record.m_parent);
  }

  inline std::vector<RegistryEntry> FileSystemRegistryDataStore::LoadChildren(
      const RegistryEntry& directory) {
    auto record = LoadRecord(directory.m_id);
    auto children = std::vector<RegistryEntry>();
    std::transform(record.m_children.begin(), record.m_children.end(),
      std::back_inserter(children), [&] (auto id) {
        return LoadRegistryEntry(id);
      });
    return children;
  }

  inline RegistryEntry FileSystemRegistryDataStore::LoadRegistryEntry(
      std::uint64_t id) {
    auto record = LoadRecord(id);
    return record.m_registryEntry;
  }

  inline RegistryEntry FileSystemRegistryDataStore::Copy(
      const RegistryEntry& source, const RegistryEntry& destination) {
    auto destinationRecord = LoadRecord(destination.m_id);
    auto sourceRecord = Details::RegistryEntryRecord();
    try {
      sourceRecord = LoadRecord(source.m_id);
    } catch(const std::exception&) {
      sourceRecord.m_registryEntry = source;
    }
    sourceRecord.m_registryEntry.m_id = LoadNextEntryId();
    auto children = std::vector<std::uint64_t>();
    children.swap(sourceRecord.m_children);
    sourceRecord.m_parent = destination.m_id;
    destinationRecord.m_children.push_back(sourceRecord.m_registryEntry.m_id);
    SaveRecord(destinationRecord);
    SaveRecord(sourceRecord);
    for(auto childId : children) {
      Copy(LoadRegistryEntry(childId), sourceRecord.m_registryEntry);
    }
    return sourceRecord.m_registryEntry;
  }

  inline void FileSystemRegistryDataStore::Move(const RegistryEntry& source,
      const RegistryEntry& destination) {
    auto sourceRecord = LoadRecord(source.m_id);
    auto destinationRecord = LoadRecord(destination.m_id);
    auto parentRecord = LoadRecord(sourceRecord.m_parent);
    sourceRecord.m_parent = destination.m_id;
    destinationRecord.m_children.push_back(sourceRecord.m_registryEntry.m_id);
    parentRecord.m_children.erase(std::find(parentRecord.m_children.begin(),
      parentRecord.m_children.end(), sourceRecord.m_registryEntry.m_id));
    SaveRecord(destinationRecord);
    SaveRecord(sourceRecord);
    SaveRecord(parentRecord);
  }

  inline void FileSystemRegistryDataStore::Delete(
      const RegistryEntry& registryEntry) {
    auto record = LoadRecord(registryEntry.m_id);
    for(auto childId : record.m_children) {
      Delete(LoadRecord(childId).m_registryEntry);
    }
    auto parent = LoadRecord(record.m_parent);
    parent.m_children.erase(std::find(parent.m_children.begin(),
      parent.m_children.end(), record.m_registryEntry.m_id));
    SaveRecord(parent);
    auto registryPath = GetPath(registryEntry.m_id);
    std::filesystem::remove(registryPath);
  }

  inline IO::SharedBuffer FileSystemRegistryDataStore::Load(
      const RegistryEntry& registryEntry) {
    auto record = LoadRecord(registryEntry.m_id);
    if(record.m_registryEntry.m_type != RegistryEntry::Type::VALUE) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException("Entry not found."));
    }
    return record.m_value;
  }

  inline RegistryEntry FileSystemRegistryDataStore::Store(
      const RegistryEntry& registryEntry, const IO::SharedBuffer& value) {
    auto record = LoadRecord(registryEntry.m_id);
    if(record.m_registryEntry.m_type != RegistryEntry::Type::VALUE) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException("Entry not found."));
    }
    record.m_value = value;
    ++record.m_registryEntry.m_version;
    SaveRecord(record);
    return record.m_registryEntry;
  }

  inline void FileSystemRegistryDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    auto lock = boost::lock_guard(m_mutex);
    transaction();
  }

  inline void FileSystemRegistryDataStore::Close() {
    m_openState.Close();
  }

  inline std::filesystem::path FileSystemRegistryDataStore::GetPath(
      std::uint64_t id) {
    std::filesystem::path registryPath =
      m_root / boost::lexical_cast<std::string>(id);
    return registryPath;
  }

  inline Details::RegistryEntryRecord FileSystemRegistryDataStore::LoadRecord(
      std::uint64_t id) {
    auto record = Details::RegistryEntryRecord();
    try {
      auto reader = IO::BasicIStreamReader<std::ifstream>(
        Initialize(GetPath(id), std::ios::binary));
      auto buffer = IO::SharedBuffer();
      reader.Read(Beam::Store(buffer));
      auto receiver = Serialization::BinaryReceiver<IO::SharedBuffer>();
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(record);
    } catch(const std::exception&) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException(
        "Unable to load entry."));
    }
    return record;
  }

  inline void FileSystemRegistryDataStore::SaveRecord(
      const Details::RegistryEntryRecord& record) {
    try {
      auto writer = IO::BasicOStreamWriter<std::ofstream>(
        Initialize(GetPath(record.m_registryEntry.m_id), std::ios::binary));
      auto buffer = IO::SharedBuffer();
      auto sender = Serialization::BinarySender<IO::SharedBuffer>();
      sender.SetSink(Ref(buffer));
      sender.Shuttle(record);
      writer.Write(buffer);
    } catch(const std::exception&) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException(
        "Unable to save entry."));
    }
  }

  inline std::uint64_t FileSystemRegistryDataStore::LoadNextEntryId() {
    auto value = std::uint64_t();
    {
      auto reader = IO::BasicIStreamReader<std::ifstream>(
        Initialize(m_root / "settings.dat", std::ios::binary));
      auto buffer = IO::SharedBuffer();
      reader.Read(Beam::Store(buffer));
      auto receiver = Serialization::BinaryReceiver<IO::SharedBuffer>();
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(value);
      ++value;
    }
    {
      auto writer = IO::BasicOStreamWriter<std::ofstream>(
        Initialize(m_root / "settings.dat", std::ios::binary));
      auto buffer = IO::SharedBuffer();
      auto sender = Serialization::BinarySender<IO::SharedBuffer>();
      sender.SetSink(Ref(buffer));
      sender.Shuttle(value);
      writer.Write(buffer);
    }
    return value;
  }
}

#endif
