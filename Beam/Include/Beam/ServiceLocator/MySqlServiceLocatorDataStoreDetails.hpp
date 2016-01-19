#ifndef BEAM_MYSQLSERVICELOCATORDATASTOREDETAILS_HPP
#define BEAM_MYSQLSERVICELOCATORDATASTOREDETAILS_HPP
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Details {
namespace SqlInsert {
  sql_create_1(settings, 1, 0,
    mysqlpp::sql_int_unsigned, next_entry_id);

  sql_create_5(accounts, 1, 5,
    mysqlpp::sql_int_unsigned, id,
    mysqlpp::sql_varchar, name,
    mysqlpp::sql_varchar, password,
    mysqlpp::sql_datetime, registration_time,
    mysqlpp::sql_datetime, last_login_time);

  sql_create_2(directories, 2, 0,
    mysqlpp::sql_int_unsigned, id,
    mysqlpp::sql_varchar, name);

  sql_create_2(parents, 2, 0,
    mysqlpp::sql_int_unsigned, entry,
    mysqlpp::sql_int_unsigned, parent);

  sql_create_2(children, 2, 0,
    mysqlpp::sql_int_unsigned, entry,
    mysqlpp::sql_int_unsigned, child);

  sql_create_3(permissions, 3, 0,
    mysqlpp::sql_int_unsigned, source,
    mysqlpp::sql_int_unsigned, target,
    mysqlpp::sql_int_unsigned, permission);
}

  sql_create_1(settings, 1, 0,
    mysqlpp::sql_int_unsigned, next_entry_id);

  sql_create_5(accounts, 1, 5,
    mysqlpp::sql_int_unsigned, id,
    mysqlpp::sql_varchar, name,
    mysqlpp::sql_varchar, password,
    mysqlpp::sql_datetime, registration_time,
    mysqlpp::sql_datetime, last_login_time);

  sql_create_2(directories, 2, 0,
    mysqlpp::sql_int_unsigned, id,
    mysqlpp::sql_varchar, name);

  sql_create_2(parents, 2, 0,
    mysqlpp::sql_int_unsigned, entry,
    mysqlpp::sql_int_unsigned, parent);

  sql_create_2(children, 2, 0,
    mysqlpp::sql_int_unsigned, entry,
    mysqlpp::sql_int_unsigned, child);

  sql_create_3(permissions, 3, 0,
    mysqlpp::sql_int_unsigned, source,
    mysqlpp::sql_int_unsigned, target,
    mysqlpp::sql_int_unsigned, permission);

  inline bool TableExists(mysqlpp::Connection& databaseConnection,
      const std::string& schema, const char* table) {
    mysqlpp::Query query = databaseConnection.query();
    query << "SHOW TABLES IN " << schema << " LIKE " << mysqlpp::quote << table;
    mysqlpp::StoreQueryResult result = query.store();
    return !result.empty();
  }

  inline bool LoadSettings(mysqlpp::Connection& databaseConnection,
      Out<unsigned int> nextEntryId) {
    mysqlpp::Query query = databaseConnection.query();
    query << "SELECT * FROM settings";
    mysqlpp::StoreQueryResult result = query.store();
    if(!result) {
      return false;
    }
    settings row = result.front();
    *nextEntryId = row.next_entry_id;
    return true;
  }

  inline bool LoadSettingsTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "settings")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE settings (next_entry_id INTEGER UNSIGNED NOT NULL)";
    if(!query.execute()) {
      return false;
    }
    query.reset();
    SqlInsert::settings settingsRow(0);
    query.insert(settingsRow);
    return query.execute();
  }

  inline bool LoadAccountsTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "accounts")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE accounts ("
      "id INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "name VARCHAR(100) BINARY NOT NULL,"
      "password VARCHAR(100) BINARY NOT NULL,"
      "registration_time DATETIME NOT NULL,"
      "last_login_time DATETIME NOT NULL)";
    return query.execute();
  }

  inline bool LoadDirectoriesTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "directories")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE directories ("
      "id INTEGER UNSIGNED PRIMARY KEY NOT NULL,"
      "name VARCHAR(100) BINARY NOT NULL)";
    return query.execute();
  }

  inline bool LoadParentsTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "parents")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE parents ("
      "entry INTEGER NOT NULL,"
      "parent INTEGER NOT NULL)";
    return query.execute();
  }

  inline bool LoadChildrenTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "children")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE children ("
      "entry INTEGER NOT NULL,"
      "child INTEGER NOT NULL)";
    return query.execute();
  }

  inline bool LoadPermissionsTable(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(TableExists(databaseConnection, schema, "permissions")) {
      return true;
    }
    mysqlpp::Query query = databaseConnection.query();
    query << "CREATE TABLE permissions ("
      "source INTEGER UNSIGNED NOT NULL,"
      "target INTEGER UNSIGNED NOT NULL,"
      "permission INTEGER UNSIGNED NOT NULL)";
    return query.execute();
  }

  inline bool LoadTables(mysqlpp::Connection& databaseConnection,
      const std::string& schema) {
    if(!LoadSettingsTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadAccountsTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadDirectoriesTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadParentsTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadChildrenTable(databaseConnection, schema)) {
      return false;
    }
    if(!LoadPermissionsTable(databaseConnection, schema)) {
      return false;
    }
    return true;
  }
}
}
}

#endif
