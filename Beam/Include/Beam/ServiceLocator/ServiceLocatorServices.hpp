#ifndef BEAM_SERVICE_LOCATOR_SERVICES_HPP
#define BEAM_SERVICE_LOCATOR_SERVICES_HPP
#include <boost/optional.hpp>
#include "Beam/Serialization/ShuttleBitset.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Serialization/ShuttleOptional.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"
#include "Beam/ServiceLocator/AccountUpdate.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"

namespace Beam::ServiceLocator {
  BEAM_DEFINE_RECORD(LoginServiceResult, DirectoryEntry, account,
    std::string, session_id);

  BEAM_DEFINE_SERVICES(ServiceLocatorServices,

    /**
     * Sends an encoded session id.
     * @param key The key used to encode the session id.
     * @param session_id The encoded session id.
     */
    (SendSessionIdService, "Beam.ServiceLocator.SendSessionIdService", void,
      unsigned int, key, std::string, session_id),

    /**
     * Logs into the ServiceLocator.
     * @param username The account's username.
     * @param password The account's password.
     * @return A record as follows:
     *         account: The DirectoryEntry identifying the account that logged
     *                  in.
     *         session_id: The login's session id.
     */
    (LoginService, "Beam.ServiceLocator.LoginService", LoginServiceResult,
      std::string, username, std::string, password),

    /**
     * Registers a service with the ServiceLocator.
     * @param name The name of the service.
     * @param properties The service's properties.
     * @return A ServiceEntry representing the registered service.
     */
    (RegisterService, "Beam.ServiceLocator.RegisterService", ServiceEntry,
      std::string, name, JsonObject, properties),

    /**
     * Unregisters a service from the ServiceLocator.
     * @param service The id of the service to unregister.
     */
    (UnregisterService, "Beam.ServiceLocator.UnregisterService", void, int,
      service),

    /**
     * Locates a service.
     * @param name The name of the service to locate.
     * @return A list of ServiceEntries for the service that was located.
     */
    (LocateService, "Beam.ServiceLocator.LocateService",
      std::vector<ServiceEntry>, std::string, name),

    /**
     * Subscribes to notifications about the availability of a service.
     * @param name The name of the service to subscribe to.
     * @return A list of ServiceEntries for the services currently available
     *         with the specified <i>name</i>.
     */
    (SubscribeAvailabilityService,
      "Beam.ServiceLocator.SubscribeAvailabilityService",
      std::vector<ServiceEntry>, std::string, name),

    /**
     * Unsubscribes from notifications about the availability of a service.
     * @param name The name of the service to unsubscribe from.
     */
    (UnsubscribeAvailabilityService,
      "Beam.ServiceLocator.UnsubscribeAvailabilityService", void, std::string,
      name),

    /**
     * Requests update messages to a DirectoryEntry.
     * @param entry The DirectoryEntry to monitor.
     * @return The list of the <i>entry</i>'s parents.
     */
    (MonitorDirectoryEntryService,
      "Beam.ServiceLocator.MonitorDirectoryEntryService",
      std::vector<DirectoryEntry>, DirectoryEntry, entry),

    /**
     * Returns all accounts and sends updates when accounts are created/removed.
     * @return The list of all accounts.
     */
    (MonitorAccountsService, "Beam.ServiceLocator.MonitorAccountsService",
      std::vector<DirectoryEntry>),

    /**
     * Stops monitoring account updates.
     */
    (UnmonitorAccountsService, "Beam.ServiceLocator.UnmonitorAccountsService",
      void),

    /**
     * Loads a DirectoryEntry from its id.
     * @param id The id of the DirectoryEntry to load.
     * @return The DirectoryEntry with the specified <i>id</i>.
     */
    (LoadDirectoryEntryService, "Beam.ServiceLocator.LoadDirectoryEntryService",
      DirectoryEntry, unsigned int, id),

    /**
     * Loads a DirectoryEntry from a path.
     * @param path The path to the DirectoryEntry to load.
     * @param root The root DirectoryEntry to search from.
     * @return The DirectoryEntry at the specified <i>path</i>.
     */
    (LoadPathService, "Beam.ServiceLocator.LoadPathService", DirectoryEntry,
      DirectoryEntry, root, std::string, path),

    /**
     * Loads all of a DirectoryEntry's parents.
     * @param entry The DirectoryEntry whose parents are to be loaded.
     * @return The list of parents.
     */
    (LoadParentsService, "Beam.ServiceLocator.LoadParentsService",
      std::vector<DirectoryEntry>, DirectoryEntry, entry),

    /**
     * Loads all of a DirectoryEntry's children.
     * @param entry The DirectoryEntry whose children are to be loaded.
     * @return The list of children.
     */
    (LoadChildrenService, "Beam.ServiceLocator.LoadChildrenService",
      std::vector<DirectoryEntry>, DirectoryEntry, entry),

    /**
     * Loads all accounts the session is permissioned to read from.
     * @return The list of all accounts the session is permissioned to read
     *         from.
     */
    (LoadAllAccountsService, "Beam.ServiceLocator.LoadAllAccountsService",
      std::vector<DirectoryEntry>),

    /**
     * Finds an account with a specified name.
     * @param name The name of the account to find.
     * @return The DirectoryEntry of the account with the specified <i>name</i>.
     */
    (FindAccountService, "Beam.ServiceLocator.FindAccountService",
      boost::optional<DirectoryEntry>, std::string, name),

    /**
     * Creates an account.
     * @param name The name of the account.
     * @param password The password.
     * @param parent The initial parent to place the DirectoryEntry.
     * @return The DirectoryEntry of the account that was created.
     */
    (MakeAccountService, "Beam.ServiceLocator.MakeAccountService",
      DirectoryEntry, std::string, name, std::string, password, DirectoryEntry,
      parent),

    /**
     * Creates a directory.
     * @param name The name of the directory.
     * @param parent The initial parent to place the DirectoryEntry.
     * @return The DirectoryEntry of the directory that was created.
     */
    (MakeDirectoryService, "Beam.ServiceLocator.MakeDirectoryService",
      DirectoryEntry, std::string, name, DirectoryEntry, parent),

    /**
     * Deletes a DirectoryEntry.
     * @param entry The DirectoryEntry to delete.
     */
    (DeleteDirectoryEntryService,
      "Beam.ServiceLocator.DeleteDirectoryEntryService", void, DirectoryEntry,
      entry),

    /**
     * Associates a DirectoryEntry with a parent.
     * @param entry The DirectoryEntry to associate.
     * @param parent The DirectoryEntry to serve as a parent of the
     *        <i>entry</i>.
     */
    (AssociateService, "Beam.ServiceLocator.AssociateService", void,
      DirectoryEntry, entry, DirectoryEntry, parent),

    /**
     * Detaches a DirectoryEntry from a parent.
     * @param entry The DirectoryEntry to detach.
     * @param parent The parent DirectoryEntry to detach.
     */
    (DetachService, "Beam.ServiceLocator.DetachService", void, DirectoryEntry,
      entry, DirectoryEntry, parent),

    /**
     * Sets an account's password.
     * @param account The account to set the password for.
     * @param password The <i>account</i>'s new password.
     */
    (StorePasswordService, "Beam.ServiceLocator.StorePasswordService", void,
      DirectoryEntry, account, std::string, password),

    /**
     * Tests if an account has Permissions to a DirectoryEntry.
     * @param account The account to test.
     * @param target The DirectoryEntry to test.
     * @param permissions The Permissions to test.
     * @return <code>true</code> iff the <i>account</i> has the specified
     *         <code>permissions</code> on the <i>target</i>.
     */
    (HasPermissionsService, "Beam.ServiceLocator.HasPermissionsService", bool,
      DirectoryEntry, account, DirectoryEntry, target, Permissions,
      permissions),

    /**
     * Sets a DirectoryEntry's Permissions to another DirectoryEntry.
     * @param source <code>DirectoryEntry</code> The DirectoryEntry being
     *        granted or revoked of Permissions.
     * @param target <code>DirectoryEntry</code> The DirectoryEntry whose
     *        Permissions are being granted or revoked from the <i>source</i>.
     * @param permissions <code>Permissions</code> The Permissions to grant the
     *        <i>source</i> over the <i>target</i>.
     */
    (StorePermissionsService, "Beam.ServiceLocator.StorePermissionsService",
      void, DirectoryEntry, source, DirectoryEntry, target, Permissions,
      permissions),

    /**
     * Loads the registration time of an account.
     * @param account The account whose registration time is to be loaded.
     * @return The <i>account</i>'s time of registration.
     */
    (LoadRegistrationTimeService,
      "Beam.ServiceLocator.LoadRegistrationTimeService",
      boost::posix_time::ptime, DirectoryEntry, account),

    /**
     * Loads an account's most recent login time.
     * @param account The account whose last login time is to be loaded.
     * @return The <i>account</i>'s most recent login time.
     */
    (LoadLastLoginTimeService, "Beam.ServiceLocator.LoadLastLoginTimeService",
      boost::posix_time::ptime, DirectoryEntry, account),

    /**
     * Renames a DirectoryEntry.
     * @param entry The DirectoryEntry to rename.
     * @param name The name to assign to the <i>entry</i>.
     * @return The updated DirectoryEntry.
     */
    (RenameService, "Beam.ServiceLocator.RenameService", DirectoryEntry,
      DirectoryEntry, entry, std::string, name),

    /**
     * Authenticates an account.
     * @param username The account's username.
     * @param password The account's password.
     * @return The account's DirectoryEntry if the <i>username</i> and
     *         <i>password</i> match.
     */
    (AuthenticateAccountService,
      "Beam.ServiceLocator.AuthenticateAccountService", DirectoryEntry,
      std::string, username, std::string, password),

    /**
     * Authenticates a session.
     * @param session_id The encrypted session id to authenticate.
     * @param key The encryption key used to encode the session id.
     * @return The DirectoryEntry associated with the session_id.
     */
    (SessionAuthenticationService,
      "Beam.ServiceLocator.SessionAuthenticationService", DirectoryEntry,
      std::string, session_id, unsigned int, key));

  BEAM_DEFINE_MESSAGES(ServiceLocatorMessages,

    /**
     * Indicates one DirectoryEntry has been associated with another.
     * @param entry The DirectoryEntry that's being associated.
     * @param parent The parent the <i>entry</i> is now associated with.
     */
    (DirectoryEntryAssociatedMessage,
      "Beam.ServiceLocator.DirectoryEntryAssociatedMessage", DirectoryEntry,
      entry, DirectoryEntry, parent),

    /**
     * Indicates one DirectoryEntry has been detached from another.
     * @param entry The DirectoryEntry that's detached.
     * @param parent The parent the <i>entry</i> is now detached from.
     */
    (DirectoryEntryDetachedMessage,
      "Beam.ServiceLocator.DirectoryEntryDetachedMessage", DirectoryEntry,
      entry, DirectoryEntry, parent),

    /**
     * Indicates whether a service is available.
     * @param service The service.
     * @param is_available <code>true</code> iff the <i>service</i> is
     *        available.
     */
    (ServiceAvailabilityMessage,
      "Beam.ServiceLocator.ServiceAvailabilityMessage", ServiceEntry, service,
      bool, is_available),

    /**
     * Indicates an account has been added or deleted.
     * @param update The update that occurred.
     */
    (AccountUpdateMessage, "Beam.ServiceLocator.AccountUpdateMessage",
      AccountUpdate, update));
}

#endif
