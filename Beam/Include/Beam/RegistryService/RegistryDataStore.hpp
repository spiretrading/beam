#ifndef BEAM_REGISTRYDATASTORE_HPP
#define BEAM_REGISTRYDATASTORE_HPP
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/RegistryService/RegistryDataStoreException.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/RegistryService/RegistryService.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class RegistryDataStore
      \brief Base class used to store RegistryService data.
   */
  class RegistryDataStore : private boost::noncopyable {
    public:
      virtual ~RegistryDataStore() = default;

      //! Loads a RegistryEntry's parent.
      /*!
        \param registryEntry The RegistryEntry whose parent is to be loaded.
        \return The <i>registryEntry</i>'s parent.
      */
      virtual RegistryEntry LoadParent(const RegistryEntry& registryEntry) = 0;

      //! Loads a directory's children.
      /*!
        \param directory The directory whose children are to be returned.
        \return The list of the <i>directory</i>'s children.
      */
      virtual std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& directory) = 0;

      //! Loads a RegistryEntry from its id.
      /*!
        \param id The id of the RegistryEntry to load.
        \return The RegistryEntry with the specified <i>id</i>.
      */
      virtual RegistryEntry LoadRegistryEntry(std::uint64_t id) = 0;

      //! Copies a RegistryEntry.
      /*!
        \param source The RegistryEntry to copy.
        \param destination Where to store the copy.
      */
      virtual RegistryEntry Copy(const RegistryEntry& source,
        const RegistryEntry& destination) = 0;

      //! Moves a RegistryEntry.
      /*!
        \param source The RegistryEntry to move.
        \param destination Where to store the <i>source</i>.
      */
      virtual void Move(const RegistryEntry& source,
        const RegistryEntry& destination) = 0;

      //! Deletes a RegistryEntry.
      /*!
        \param registryEntry The RegistryEntry to delete.
      */
      virtual void Delete(const RegistryEntry& registryEntry) = 0;

      //! Loads a registry value.
      /*!
        \param registryEntry The RegistryEntry of the value to load.
        \return The value stored.
      */
      virtual IO::SharedBuffer Load(const RegistryEntry& registryEntry) = 0;

      //! Stores a registry value.
      /*!
        \param registryEntry The RegistryEntry to store the value into.
        \param value The value to store.
        \return The updated RegistryEntry.
      */
      virtual RegistryEntry Store(const RegistryEntry& registryEntry,
        const IO::SharedBuffer& value) = 0;

      //! Validates a RegistryEntry.
      /*!
        \param entry The RegistryEntry to validate.
        \return The RegistryEntry as it's stored in this data store.
      */
      virtual RegistryEntry Validate(const RegistryEntry& entry);

      //! Performs an atomic transaction.
      /*!
        \param transaction The transaction to perform.
      */
      virtual void WithTransaction(
        const std::function<void ()>& transaction) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;
  };

  //! Returns the RegistryEntry at a specified path.
  /*!
    \param dataStore The RegistryDataStore to search for the path.
    \param root The root RegistryEntry to begin searching from.
    \param path The path to search for.
    \return The RegistryEntry at the specified <i>path</i>.
  */
  inline RegistryEntry LoadRegistryEntry(RegistryDataStore& dataStore,
      const RegistryEntry& root, const std::string& path) {
    if(path.empty()) {
      return root;
    }
    std::string::size_type delimiter = path.find('/');
    std::string segment;
    if(delimiter == std::string::npos) {
      segment = path;
    } else {
      segment = path.substr(0, delimiter);
    }
    std::vector<RegistryEntry> children = dataStore.LoadChildren(root);
    for(const RegistryEntry& child : children) {
      if(child.m_name == segment) {
        if(delimiter == std::string::npos) {
          return child;
        } else {
          return LoadRegistryEntry(dataStore, child,
            path.substr(delimiter + 1));
        }
      }
    }
    BOOST_THROW_EXCEPTION(RegistryDataStoreException{
      "Registry entry not found."});
  }

  inline RegistryEntry RegistryDataStore::Validate(const RegistryEntry& entry) {
    RegistryEntry validatedEntry = LoadRegistryEntry(entry.m_id);
    if(validatedEntry != entry) {
      BOOST_THROW_EXCEPTION(RegistryDataStoreException{
        "Registry entry not found."});
    }
    return validatedEntry;
  }
}
}

#endif
