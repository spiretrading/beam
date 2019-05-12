#ifndef BEAM_SQL_SERVICE_LOCATOR_DATA_STORE_HPP
#define BEAM_SQL_SERVICE_LOCATOR_DATA_STORE_HPP
#include <boost/throw_exception.hpp>
#include <Viper/Viper.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/ServiceLocator/SqlDefinitions.hpp"
#include "Beam/Sql/Utilities.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::ServiceLocator {

  /** Implements the ServiceLocatorDataStore using SQL.
      \tparam C The type of SQL connection to use.
   */
  template<typename C>
  class SqlServiceLocatorDataStore : public ServiceLocatorDataStore {
    public:

      //! The type of SQL connection to use.
      using Connection = C;

      //! Constructs an SqlServiceLocatorDataStore.
      /*!
        \param connection The SQL connection to use.
      */
      SqlServiceLocatorDataStore(std::unique_ptr<Connection> connection);

      ~SqlServiceLocatorDataStore() override;

      std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override;

      std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& directory) override;

      DirectoryEntry LoadDirectoryEntry(unsigned int id) override;

      std::vector<DirectoryEntry> LoadAllAccounts() override;

      std::vector<DirectoryEntry> LoadAllDirectories() override;

      DirectoryEntry LoadAccount(const std::string& name) override;

      DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password, const DirectoryEntry& parent,
        const boost::posix_time::ptime& registrationTime) override;

      DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) override;

      void Delete(const DirectoryEntry& entry) override;

      bool Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      bool Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override;

      std::string LoadPassword(const DirectoryEntry& account) override;

      void SetPassword(const DirectoryEntry& account,
        const std::string& password) override;

      Permissions LoadPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target) override;

      std::vector<std::tuple<DirectoryEntry, Permissions>>
        LoadAllPermissions(const DirectoryEntry& account) override;

      void SetPermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override;

      boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override;

      boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override;

      void StoreLastLoginTime(const DirectoryEntry& account,
        const boost::posix_time::ptime& loginTime) override;

      void Rename(const DirectoryEntry& entry,
        const std::string& name) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Open() override;

      void Close() override;

    private:
      mutable Threading::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      unsigned int m_nextEntryId;
      IO::OpenState m_openState;

      void Shutdown();
      unsigned int LoadNextEntryId();
  };

  template<typename C>
  SqlServiceLocatorDataStore<C>::SqlServiceLocatorDataStore(
      std::unique_ptr<Connection> connection)
      : m_connection(std::move(connection)) {}

  template<typename C>
  SqlServiceLocatorDataStore<C>::~SqlServiceLocatorDataStore() {
    Close();
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::LoadParents(
      const DirectoryEntry& entry) {
    auto parentIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select({"parent"}, "parents",
        Viper::sym("entry") == entry.m_id, std::back_inserter(parentIds));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto parents = std::vector<DirectoryEntry>();
    for(auto& parentId : parentIds) {
      parents.push_back(LoadDirectoryEntry(parentId));
    }
    return parents;
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::LoadChildren(
      const DirectoryEntry& entry) {
    auto childIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select({"child"}, "children",
        Viper::sym("entry") == entry.m_id, std::back_inserter(childIds));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto children = std::vector<DirectoryEntry>();
    for(auto& childId : childIds) {
      children.push_back(LoadDirectoryEntry(childId));
    }
    return children;
  }

  inline DirectoryEntry MySqlServiceLocatorDataStore::LoadDirectoryEntry(
      unsigned int id) {
    auto query = m_databaseConnection.query();
    query << "SELECT type, id, name FROM "
      "(SELECT " << DirectoryEntry::Type::ACCOUNT << " AS type, id, name "
      "FROM accounts UNION ALL SELECT " << DirectoryEntry::Type::DIRECTORY <<
      " AS type, id, name FROM directories) AS directory_entries WHERE id = " <<
      id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    if(result.empty()) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException{"Directory entry not found."});
    }
    assert(result.size() == 1);
    DirectoryEntry entry{static_cast<DirectoryEntry::Type>(
      static_cast<int>(result[0][0])), static_cast<unsigned int>(result[0][1]),
      result[0][2].c_str()};
    return entry;
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::LoadAllAccounts() {
    auto accountIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select({"*"}, "accounts",
        std::back_inserter(accountIds)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto accounts = std::vector<DirectoryEntry>();
    for(auto& accountId : accountIds) {
      accounts.push_back(LoadDirectoryEntry(accountId));
    }
    return accounts;
  }

  template<typename C>
  std::vector<DirectoryEntry> SqlServiceLocatorDataStore<C>::
      LoadAllDirectories() {
    auto directoryIds = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select({"*"}, "directories",
        std::back_inserter(directoryIds)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto directories = std::vector<DirectoryEntry>();
    for(auto& directoryId : directoryIds) {
      directories.push_back(LoadDirectoryEntry(directoryId));
    }
    return directories;
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::LoadAccount(
      const std::string& name) {
    auto ids = std::vector<unsigned int>();
    try {
      m_connection->execute(Viper::select({"id"}, "accounts",
        Viper::sym("name") == name, std::back_inserter(ids)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    if(ids.empty()) {
      BOOST_THROW_EXCEPTION(
        ServiceLocatorDataStoreException("Account not found."));
    }
    assert(ids.size() == 1);
    return DirectoryEntry::MakeAccount(ids.front(), name);
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::MakeAccount(
      const std::string& name, const std::string& password,
      const DirectoryEntry& parent,
      const boost::posix_time::ptime& registrationTime) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent must be a directory."));
    }
    auto accountExists = [&] {
      try {
        LoadAccount(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(accountExists) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto entry = DirectoryEntry::MakeAccount(LoadNextEntryId(), name);
    auto row = AccountsRow{entry, HashPassword(entry, password),
      registrationTime, boost::posix_time::neg_infin};
    try {
      m_connection->execute(Viper::insert(GetAccountsRow(), "accounts", &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    Associate(entry, parent);
    return entry;
  }

  template<typename C>
  DirectoryEntry SqlServiceLocatorDataStore<C>::MakeDirectory(
      const std::string& name, const DirectoryEntry& parent) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent must be a directory."));
    }
    auto accountExists = [&] {
      try {
        LoadAccount(name);
        return true;
      } catch(const ServiceLocatorDataStoreException&) {
        return false;
      }
    }();
    if(accountExists) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "An account with the specified name exists."));
    }
    auto entry = DirectoryEntry::MakeDirectory(LoadNextEntryId(), name);
    try {
      m_connection->execute(Viper::insert(GetDirectoriesRow(), "directories",
        &entry));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    Associate(entry, parent);
    return entry;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::Delete(const DirectoryEntry& entry) {
    auto children = LoadChildren(entry);
    if(!children.empty()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Directory entry is not empty."));
    }
    try {
      m_connection->execute(Viper::erase("permissions",
        Viper::sym("source") == entry.m_id ||
        Viper::sym("target") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    auto parents = LoadParents(entry);
    for(auto& parent : parents) {
      Detach(entry, parent);
    }
    auto table = [&] {
      if(entry.m_type == DirectoryEntry::Type::ACCOUNT) {
        return "accounts";
      } else {
        return "directories";
      }
    }();
    try {
      m_connection->execute(Viper::erase(table,
        Viper::sym("id") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  bool SqlServiceLocatorDataStore<C>::Associate(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(parent.m_id == -1) {
      return false;
    }
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent not found."));
    }
    if(entry == parent) {
      return false;
    }
    auto parents = LoadParents(entry);
    if(std::find(parents.begin(), parents.end(), parent) != parents.end()) {
      return false;
    }
    try {
      auto parentsRow = ParentsRow{entry.m_id, parent.m_id};
      m_connection->execute(Viper::insert(GetParentsRow(), "parents",
        &parentsRow));
      auto childrensRow = ChildrensRow{parent.m_id, entry.m_id};
      m_connection->execute(Viper::insert(GetChildrensRow(), "children",
        &childrensRow));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    return true;
  }

  template<typename C>
  bool SqlServiceLocatorDataStore<C>::Detach(const DirectoryEntry& entry,
      const DirectoryEntry& parent) {
    if(parent.m_type != DirectoryEntry::Type::DIRECTORY) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Parent not found."));
    }
    if(entry == parent) {
      return false;
    }
    try {
      m_connection->execute(Viper::erase("parents",
        Viper::sym("entry") == entry.m_id &&
        Viper::sym("parent") == parent.m_id));
      m_connection->execute(Viper::erase("children",
        Viper::sym("entry") == parent.m_id &&
        Viper::sym("child") == entry.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    return true;
  }

  template<typename C>
  std::string SqlServiceLocatorDataStore<C>::LoadPassword(
      const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    auto password = std::string();
    try {
      m_connection->execute(Viper::select({"password"}, "accounts",
        Viper::sym("id") == account.m_id, &password));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
    return password;
  }

  template<typename C>
  void SqlServiceLocatorDataStore<C>::SetPassword(const DirectoryEntry& account,
      const std::string& password) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(
        "Account not found."));
    }
    try {
      m_connection->execute(Viper::update("accounts",
        {Viper::sym("password"), password}, Viper::sym("id") == account.m_id));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException(e.what()));
    }
  }

  template<typename C>
  Permissions SqlServiceLocatorDataStore<C>::LoadPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target) {
    auto query = m_databaseConnection.query();
    query << "SELECT permission FROM permissions WHERE source = " <<
      source.m_id << " AND target = " << target.m_id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    if(result.empty()) {
      return Permission::NONE;
    }
    return Permissions::FromRepresentation(result[0][0].conv<int>(0));
  }

  inline std::vector<std::tuple<DirectoryEntry, Permissions>>
      MySqlServiceLocatorDataStore::LoadAllPermissions(
      const DirectoryEntry& account) {
    std::vector<std::tuple<DirectoryEntry, Permissions>> permissions;
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM permissions WHERE source = " << account.m_id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    for(mysqlpp::StoreQueryResult::const_iterator i = result.begin();
        i != result.end(); ++i) {
      permissions.emplace_back(LoadDirectoryEntry(static_cast<int>((*i)[1])),
        Permissions::FromRepresentation((*i)[2].conv<int>(0)));
    }
    return permissions;
  }

  inline void MySqlServiceLocatorDataStore::SetPermissions(
      const DirectoryEntry& source, const DirectoryEntry& target,
      Permissions permissions) {
    auto query = m_databaseConnection.query();
    query << "SELECT * FROM permissions WHERE source = " << source.m_id <<
      " AND target = " << target.m_id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    query.reset();
    if(result.empty()) {
      Details::SqlInsert::permissions permissionRow{source.m_id, target.m_id,
        static_cast<mysqlpp::sql_int_unsigned>(
        permissions.GetBitset().to_ulong())};
      query.insert(permissionRow);
    } else {
      query << "UPDATE permissions SET permission = " <<
        permissions.GetBitset().to_ulong() << " WHERE source = " <<
        source.m_id << " AND target = " << target.m_id;
    }
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
  }

  inline boost::posix_time::ptime MySqlServiceLocatorDataStore::
      LoadRegistrationTime(const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{
        "Account not found."});
    }
    auto query = m_databaseConnection.query();
    query << "SELECT registration_time FROM accounts WHERE id = " <<
      account.m_id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    assert(result.size() == 1);
    return FromDateTime(static_cast<mysqlpp::DateTime>(result[0][0]));
  }

  inline boost::posix_time::ptime MySqlServiceLocatorDataStore::
      LoadLastLoginTime(const DirectoryEntry& account) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{
        "Account not found."});
    }
    auto query = m_databaseConnection.query();
    query << "SELECT last_login_time FROM accounts WHERE id = " << account.m_id;
    auto result = query.store();
    if(!result) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    assert(result.size() == 1);
    return FromDateTime(static_cast<mysqlpp::DateTime>(result[0][0]));
  }

  inline void MySqlServiceLocatorDataStore::StoreLastLoginTime(
      const DirectoryEntry& account,
      const boost::posix_time::ptime& loginTime) {
    if(account.m_type != DirectoryEntry::Type::ACCOUNT) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{
        "Account not found."});
    }
    auto query = m_databaseConnection.query();
    query << "UPDATE accounts SET last_login_time = " << mysqlpp::quote <<
      ToDateTime(loginTime) << " WHERE id = " << account.m_id;
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
  }

  inline void MySqlServiceLocatorDataStore::Rename(const DirectoryEntry& entry,
      const std::string& name) {
    {
      auto query = m_databaseConnection.query();
      query << "UPDATE accounts SET name = " << mysqlpp::quote << name <<
        " WHERE id = " << entry.m_id;
      if(!query.execute()) {
        BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
      }
    }
    {
      auto query = m_databaseConnection.query();
      query << "UPDATE directories SET name = " << mysqlpp::quote << name <<
        " WHERE id = " << entry.m_id;
      if(!query.execute()) {
        BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
      }
    }
  }

  inline void MySqlServiceLocatorDataStore::WithTransaction(
      const std::function<void ()>& transaction) {
    boost::lock_guard<Threading::Mutex> lock{m_mutex};
    mysqlpp::Transaction t{m_databaseConnection};
    try {
      transaction();
    } catch(...) {
      t.rollback();
      throw;
    }
    t.commit();
  }

  inline void MySqlServiceLocatorDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      auto connectionResult = m_databaseConnection.set_option(
        new mysqlpp::ReconnectOption{true});
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(
          IO::IOException{"Unable to set MySQL reconnect option."});
        return;
      }
      connectionResult = m_databaseConnection.connect(m_schema.c_str(),
        m_address.GetHost().c_str(), m_username.c_str(), m_password.c_str(),
        m_address.GetPort());
      if(!connectionResult) {
        BOOST_THROW_EXCEPTION(IO::ConnectException{std::string(
          "Unable to connect to MySQL database - ") +
          m_databaseConnection.error()});
      }
      if(!Details::LoadTables(m_databaseConnection, m_schema)) {
        BOOST_THROW_EXCEPTION(
          IO::IOException{"Unable to load database tables."});
      }
      if(!Details::LoadSettings(m_databaseConnection, Store(m_nextEntryId))) {
        BOOST_THROW_EXCEPTION(IO::IOException{
          "Unable to load database settings."});
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void MySqlServiceLocatorDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void MySqlServiceLocatorDataStore::Shutdown() {
    m_databaseConnection.disconnect();
    m_openState.SetClosed();
  }

  inline unsigned int MySqlServiceLocatorDataStore::LoadNextEntryId() {
    auto result = m_nextEntryId;
    auto query = m_databaseConnection.query();
    query << "UPDATE settings SET next_entry_id = " << (m_nextEntryId + 1);
    if(!query.execute()) {
      BOOST_THROW_EXCEPTION(ServiceLocatorDataStoreException{query.error()});
    }
    ++m_nextEntryId;
    return result;
  }
}
}

#endif
