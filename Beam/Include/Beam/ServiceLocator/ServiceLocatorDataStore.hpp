#ifndef BEAM_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_SERVICE_LOCATOR_DATA_STORE_HPP
#include <string>
#include <unordered_set>
#include <boost/noncopyable.hpp>
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Utilities/Bcrypt.hpp"

namespace Beam::ServiceLocator {

  /** Base class used to store ServiceLocatorClient data. */
  class ServiceLocatorDataStore : private boost::noncopyable {
    public:
      virtual ~ServiceLocatorDataStore() = default;

      /**
       * Loads all parents of a DirectoryEntry.
       * @param entry The DirectoryEntry whose parents are to be loaded.
       * @return All parents of the specified <i>entry</i>.
       */
      virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) = 0;

      /**
       * Loads a directory's children.
       * @param directory The Directory whose children are to be returned.
       * @return The list of the <i>directory</i>'s children.
       */
      virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& directory) = 0;

      /**
       * Loads a DirectoryEntry from its id.
       * @param id The id of the DirectoryEntry to load.
       * @return The DirectoryEntry with the specified <i>id</i>.
       */
      virtual DirectoryEntry LoadDirectoryEntry(unsigned int id) = 0;

      /**
       * Loads all accounts.
       * @return The list of all DirectoryEntries representing accounts.
       */
      virtual std::vector<DirectoryEntry> LoadAllAccounts() = 0;

      /**
       * Loads all directories.
       * @return The list of all DirectoryEntries representing directories.
       */
      virtual std::vector<DirectoryEntry> LoadAllDirectories() = 0;

      /**
       * Loads an account from its name.
       * @param name The name of the account.
       * @return The DirectoryEntry of the account with the specified
       *         <i>name</i>.
       */
      virtual DirectoryEntry LoadAccount(const std::string& name) = 0;

      /**
       * Creates an account.
       * @param name The name of the account.
       * @param password The account's password.
       * @param parent The initial parent to place the DirectoryEntry in.
       * @param registrationTime The time of registration.
       * @return The DirectoryEntry of the account that was created.
       */
      virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        boost::posix_time::ptime registrationTime) = 0;

      /**
       * Creates a directory.
       * @param name The name of the directory.
       * @param parent The initial parent to place the DirectoryEntry in.
       * @return The DirectoryEntry of the account that was created.
       */
      virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) = 0;

      /**
       * Deletes a DirectoryEntry.
       * @param entry The DirectoryEntry to delete.
       */
      virtual void Delete(const DirectoryEntry& entry) = 0;

      /**
       * Associates a DirectoryEntry with a parent.
       * @param entry The DirectoryEntry to associate.
       * @param parent The parent to associate the <i>entry</i> with.
       * @return <code>true</code> iff the <i>entry</i> was associated with
       *         <i>parent</i>.
       */
      virtual bool Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) = 0;

      /**
       * Detaches a DirectoryEntry from one of its parents.
       * @param entry The DirectoryEntry to detach.
       * @param parent The parent to detach the <i>entry</i> from.
       * @return <code>true</code> iff the <i>entry</i> was detached from
       *         <i>parent</i>.
       */
      virtual bool Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) = 0;

      /**
       * Loads an account's password.
       * @param account The account whose password is to be loaded.
       * @return The password associated with the <i>account</i>.
       */
      virtual std::string LoadPassword(const DirectoryEntry& account) = 0;

      /**
       * Sets an account's password.
       * @param account The account to set the password for.
       * @param password The new password.
       */
      virtual void SetPassword(const DirectoryEntry& account,
        const std::string& password) = 0;

      /**
       * Loads the Permissions a DirectoryEntry has over another.
       * @param source The DirectoryEntry to check.
       * @param target The DirectoryEntry to check.
       * @return The <i>source</i>'s Permissions over the <i>target</i>.
       */
      virtual Permissions LoadPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target) = 0;

      /**
       * Loads all of the Permissions an account has.
       * @param account The DirectoryEntry to check.
       * @return The list of Permissions the <i>account</i> has.
       */
      virtual std::vector<std::tuple<DirectoryEntry, Permissions>>
        LoadAllPermissions(const DirectoryEntry& account) = 0;

      /**
       * Sets the Permissions of one DirectoryEntry over another.
       * @param source The DirectoryEntry to grant permissions to.
       * @param target The DirectoryEntry to grant permissions over.
       * @param permissions The Permissions to grant the <i>source</i> over the
       *        <i>target</i>.
       */
      virtual void SetPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) = 0;

      /**
       * Loads the registration time of an account.
       * @param account The account whose registration time is to be loaded.
       * @return The time of the <i>account</i>'s registration.
       */
      virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) = 0;

      /**
       * Loads the time of an account's most recent login.
       * @param account The account that logged in.
       * @return The time of the <i>account</i>'s most recent login.
       */
      virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) = 0;

      /**
       * Stores the time of an account's most recent login.
       * @param account The account that logged in.
       * @param loginTime The time of the <i>account</i>'s most recent login.
       */
      virtual void StoreLastLoginTime(const DirectoryEntry& account,
        boost::posix_time::ptime loginTime) = 0;

      /**
       * Renames a DirectoryEntry.
       * @param entry The DirectoryEntry to rename.
       * @param name The name to assign to the <i>entry</i>.
       */
      virtual void Rename(const DirectoryEntry& entry,
        const std::string& name) = 0;

      /**
       * Validates a DirectoryEntry.
       * @param entry The DirectoryEntry to validate.
       * @return The DirectoryEntry as it's stored in this data store.
       */
      virtual DirectoryEntry Validate(const DirectoryEntry& entry);

      /**
       * Performs an atomic transaction.
       * @param transaction The transaction to perform.
       */
      virtual void WithTransaction(
        const std::function<void ()>& transaction) = 0;

      virtual void Close() = 0;
  };

  /**
   * Returns <code>true</code> if a DirectoryEntry has a given permission.
   * @param dataStore The ServiceLocatorDataStore storing the permissions.
   * @param source The DirectoryEntry to check.
   * @param target The DirectoryEntry to check.
   * @param permissions The permissions to test for.
   * @return <code>true</code> iff the <i>source</i> has the specified
   *         <i>permissions</i> over the <i>target</i>.
   */
  inline bool HasPermission(ServiceLocatorDataStore& dataStore,
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    struct HasPermissionHelper {
      bool operator()(ServiceLocatorDataStore& dataStore,
          const DirectoryEntry& source, const DirectoryEntry& target,
          Permissions permissions,
          std::unordered_set<DirectoryEntry>& visitedEntries) const {
        if(!visitedEntries.insert(target).second) {
          return false;
        }
        auto accountPermissions = dataStore.LoadPermissions(source, target);
        if((accountPermissions & permissions) == permissions) {
          return true;
        }
        auto parents = dataStore.LoadParents(target);
        if(parents.empty()) {
          return false;
        }
        for(auto& parent : parents) {
          if(HasPermissionHelper()(dataStore, source, parent, permissions,
              visitedEntries)) {
            return true;
          }
        }
        return false;
      }
    };
    if(source == target &&
        ((permissions & Permissions(Permission::READ)) == permissions)) {
      return true;
    }
    auto visitedEntries  = std::unordered_set<DirectoryEntry>();
    return HasPermissionHelper()(dataStore, source, target, permissions,
      visitedEntries);
  }

  /**
   * Returns the DirectoryEntry at a specified path.
   * @param dataStore The ServiceLocatorDataStore to search for the path.
   * @param root The root DirectoryEntry to begin searching from.
   * @param path The path to search for.
   * @return The DirectoryEntry at the specified <i>path</i>.
   */
  inline DirectoryEntry LoadDirectoryEntry(ServiceLocatorDataStore& dataStore,
      const DirectoryEntry& root, const std::string& path) {
    if(path.empty()) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException("Directory entry not found."));
    }
    auto delimiter = path.find('/');
    auto segment = std::string();
    if(delimiter == std::string::npos) {
      segment = path;
    } else {
      segment = path.substr(0, delimiter);
    }
    auto children = dataStore.LoadChildren(root);
    for(auto& child : children) {
      if(child.m_name == segment) {
        if(delimiter == std::string::npos) {
          return child;
        } else {
          return LoadDirectoryEntry(dataStore, child,
            path.substr(delimiter + 1));
        }
      }
    }
    BOOST_THROW_EXCEPTION(
      ServiceLocatorDataStoreException("Directory entry not found."));
  }

  /**
   * Builds a hashed password from a DirectoryEntry and a plain-text password.
   * @param account The account to make the password for.
   * @param password The plain-text password to hash.
   * @return A hashed password for the <i>account</i>.
   */
  inline std::string HashPassword(const DirectoryEntry& account,
      const std::string& password) {
    return BCrypt(password);
  }

  /**
   * Validates a password.
   * @param account The account to validate the password for.
   * @param receivedPassword The password received from the client.
   * @param storedPassword The password stored for the <i>account</i>.
   * @return <code>true</code> iff the <i>receivedPassword<i> matches the
   *         <i>storedPassword</i>.
   */
  inline bool ValidatePassword(const DirectoryEntry& account,
      const std::string& receivedPassword, const std::string& storedPassword) {
    if(!storedPassword.empty() && storedPassword[0] == '$') {
      return BCryptMatches(receivedPassword, storedPassword);
    }
    auto receivedPasswordHash = ComputeSHA(std::to_string(account.m_id) +
      receivedPassword);
    return receivedPasswordHash == storedPassword;
  }

  inline DirectoryEntry ServiceLocatorDataStore::Validate(
      const DirectoryEntry& entry) {
    auto validatedEntry = LoadDirectoryEntry(entry.m_id);
    if(validatedEntry != entry) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Directory entry not found."));
    }
    return validatedEntry;
  }
}

#endif
