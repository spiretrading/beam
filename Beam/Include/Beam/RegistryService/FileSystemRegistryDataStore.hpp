#ifndef BEAM_FILESYSTEMREGISTRYDATASTORE_HPP
#define BEAM_FILESYSTEMREGISTRYDATASTORE_HPP
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
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

namespace Beam {
namespace RegistryService {

  /*! \class FileSystemRegistryDataStore
      \brief Implements the RegistryDataStore using the local file system.
   */
  class FileSystemRegistryDataStore : public RegistryDataStore {
    public:

      //! Constructs a FileSystemRegistryDataStore.
      /*!
        \param root The directory storing the files.
      */
      FileSystemRegistryDataStore(const boost::filesystem::path& root);

      virtual ~FileSystemRegistryDataStore() override;

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
      boost::filesystem::path m_root;
      std::uint64_t m_nextId;
      IO::OpenState m_openState;

      boost::filesystem::path GetPath(std::uint64_t id);
      Details::RegistryEntryRecord LoadRecord(std::uint64_t id);
      void SaveRecord(const Details::RegistryEntryRecord& record);
      std::uint64_t LoadNextEntryId();
  };

  inline FileSystemRegistryDataStore::FileSystemRegistryDataStore(
      const boost::filesystem::path& root)
      : m_root{root} {}

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
    std::vector<RegistryEntry> children;
    std::transform(record.m_children.begin(), record.m_children.end(),
      std::back_inserter(children),
      [&] (auto id) {
        return this->LoadRegistryEntry(id);
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
    Details::RegistryEntryRecord sourceRecord;
    try {
      sourceRecord = LoadRecord(source.m_id);
    } catch(const std::exception&) {
      sourceRecord.m_registryEntry = source;
    }
    sourceRecord.m_registryEntry.m_id = LoadNextEntryId();
    std::vector<std::uint64_t> children;
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
    boost::filesystem::remove(registryPath);
  }

  inline IO::SharedBuffer FileSystemRegistryDataStore::Load(
      const RegistryEntry& registryEntry) {
    auto record = LoadRecord(registryEntry.m_id);
    if(record.m_registryEntry.m_type != RegistryEntry::Type::VALUE) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{"Entry not found."});
    }
    return record.m_value;
  }

  inline RegistryEntry FileSystemRegistryDataStore::Store(
      const RegistryEntry& registryEntry, const IO::SharedBuffer& value) {
    auto record = LoadRecord(registryEntry.m_id);
    if(record.m_registryEntry.m_type != RegistryEntry::Type::VALUE) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{"Entry not found."});
    }
    record.m_value = value;
    ++record.m_registryEntry.m_version;
    SaveRecord(record);
    return record.m_registryEntry;
  }

  inline void FileSystemRegistryDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    transaction();
  }

  inline void FileSystemRegistryDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    boost::filesystem::create_directories(m_root);
    if(!boost::filesystem::exists(m_root / "settings.dat")) {
      IO::BasicOStreamWriter<boost::filesystem::ofstream> writer(
        Initialize(m_root / "settings.dat", std::ios::binary));
      IO::SharedBuffer buffer;
      Serialization::BinarySender<IO::SharedBuffer> sender;
      sender.SetSink(Ref(buffer));
      sender.Send(std::uint64_t(1));
      writer.Write(buffer);
    }
    try {
      auto root = LoadRegistryEntry(RegistryEntry::GetRoot().m_id);
    } catch(const std::exception&) {
      Details::RegistryEntryRecord rootRecord;
      rootRecord.m_registryEntry = RegistryEntry::GetRoot();
      rootRecord.m_parent = 0;
      SaveRecord(rootRecord);
    }
    m_openState.SetOpen();
  }

  inline void FileSystemRegistryDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }

  inline boost::filesystem::path FileSystemRegistryDataStore::GetPath(
      std::uint64_t id) {
    boost::filesystem::path registryPath =
      m_root / boost::lexical_cast<std::string>(id);
    return registryPath;
  }

  inline Details::RegistryEntryRecord FileSystemRegistryDataStore::LoadRecord(
      std::uint64_t id) {
    Details::RegistryEntryRecord record;
    try {
      IO::BasicIStreamReader<boost::filesystem::ifstream> reader(
        Initialize(GetPath(id), std::ios::binary));
      IO::SharedBuffer buffer;
      reader.Read(Beam::Store(buffer));
      Serialization::BinaryReceiver<IO::SharedBuffer> receiver;
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(record);
    } catch(const std::exception&) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Unable to load entry."});
    }
    return record;
  }

  inline void FileSystemRegistryDataStore::SaveRecord(
      const Details::RegistryEntryRecord& record) {
    try {
      IO::BasicOStreamWriter<boost::filesystem::ofstream> writer(
        Initialize(GetPath(record.m_registryEntry.m_id), std::ios::binary));
      IO::SharedBuffer buffer;
      Serialization::BinarySender<IO::SharedBuffer> sender;
      sender.SetSink(Ref(buffer));
      sender.Shuttle(record);
      writer.Write(buffer);
    } catch(const std::exception&) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Unable to save entry."});
    }
  }

  inline std::uint64_t FileSystemRegistryDataStore::LoadNextEntryId() {
    std::uint64_t value;
    {
      IO::BasicIStreamReader<boost::filesystem::ifstream> reader(
        Initialize(m_root / "settings.dat", std::ios::binary));
      IO::SharedBuffer buffer;
      reader.Read(Beam::Store(buffer));
      Serialization::BinaryReceiver<IO::SharedBuffer> receiver;
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(value);
      ++value;
    }
    {
      IO::BasicOStreamWriter<boost::filesystem::ofstream> writer(
        Initialize(m_root / "settings.dat", std::ios::binary));
      IO::SharedBuffer buffer;
      Serialization::BinarySender<IO::SharedBuffer> sender;
      sender.SetSink(Ref(buffer));
      sender.Shuttle(value);
      writer.Write(buffer);
    }
    return value;
  }
}
}

#endif
